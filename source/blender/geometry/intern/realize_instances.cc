/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "GEO_realize_instances.hh"

#include "DNA_collection_types.h"
#include "DNA_layer_types.h"
#include "DNA_mesh_types.h"
#include "DNA_meshdata_types.h"
#include "DNA_object_types.h"
#include "DNA_pointcloud_types.h"

#include "BLI_noise.hh"
#include "BLI_task.hh"

#include "BKE_collection.h"
#include "BKE_curves.hh"
#include "BKE_geometry_set_instances.hh"
#include "BKE_material.h"
#include "BKE_mesh.h"
#include "BKE_pointcloud.h"
#include "BKE_type_conversions.hh"

namespace blender::geometry {

using blender::bke::AttributeIDRef;
using blender::bke::custom_data_type_to_cpp_type;
using blender::bke::CustomDataAttributes;
using blender::bke::object_get_evaluated_geometry_set;
using blender::bke::OutputAttribute;
using blender::bke::OutputAttribute_Typed;
using blender::bke::ReadAttributeLookup;

/**
 * An ordered set of attribute ids. Attributes are ordered to avoid name lookups in many places.
 * Once the attributes are ordered, they can just be referred to by index.
 */
struct OrderedAttributes {
  VectorSet<AttributeIDRef> ids;
  Vector<AttributeKind> kinds;

  int size() const
  {
    return this->kinds.size();
  }

  IndexRange index_range() const
  {
    return this->kinds.index_range();
  }
};

struct AttributeFallbacksArray {
  /**
   * Instance attribute values used as fallback when the geometry does not have the
   * corresponding attributes itself. The pointers point to attributes stored in the instances
   * component or in #r_temporary_arrays. The order depends on the corresponding #OrderedAttributes
   * instance.
   */
  Array<const void *> array;

  AttributeFallbacksArray(int size) : array(size, nullptr)
  {
  }
};

struct PointCloudRealizeInfo {
  const PointCloud *pointcloud = nullptr;
  /** Matches the order stored in #AllPointCloudsInfo.attributes. */
  Array<std::optional<GVArray_GSpan>> attributes;
  /** Id attribute on the point cloud. If there are no ids, this #Span is empty. */
  Span<int> stored_ids;
};

struct RealizePointCloudTask {
  /** Starting index in the final realized point cloud. */
  int start_index;
  /** Preprocessed information about the point cloud. */
  const PointCloudRealizeInfo *pointcloud_info;
  /** Transformation that is applied to all positions. */
  float4x4 transform;
  AttributeFallbacksArray attribute_fallbacks;
  /** Only used when the output contains an output attribute. */
  uint32_t id = 0;
};

/** Start indices in the final output mesh. */
struct MeshElementStartIndices {
  int vertex = 0;
  int edge = 0;
  int poly = 0;
  int loop = 0;
};

struct MeshRealizeInfo {
  const Mesh *mesh = nullptr;
  /** Maps old material indices to new material indices. */
  Array<int> material_index_map;
  /** Matches the order in #AllMeshesInfo.attributes. */
  Array<std::optional<GVArray_GSpan>> attributes;
  /** Vertex ids stored on the mesh. If there are no ids, this #Span is empty. */
  Span<int> stored_vertex_ids;
};

struct RealizeMeshTask {
  MeshElementStartIndices start_indices;
  const MeshRealizeInfo *mesh_info;
  /** Transformation that is applied to all positions. */
  float4x4 transform;
  AttributeFallbacksArray attribute_fallbacks;
  /** Only used when the output contains an output attribute. */
  uint32_t id = 0;
};

struct RealizeCurveInfo {
  const Curves *curves;
  /**
   * Matches the order in #AllCurvesInfo.attributes.
   */
  Array<std::optional<GVArray_GSpan>> attributes;

  /** ID attribute on the curves. If there are no ids, this #Span is empty. */
  Span<int> stored_ids;

  /**
   * Handle position attributes must be transformed along with positions. Accessing them in
   * advance isn't necessary theoretically, but is done to simplify other code and to avoid
   * some overhead.
   */
  Span<float3> handle_left;
  Span<float3> handle_right;

  /**
   * The radius attribute must be filled with a default of 1.0 if it
   * doesn't exist on some (but not all) of the input curves data-blocks.
   */
  Span<float> radius;
};

/** Start indices in the final output curves data-block. */
struct CurvesElementStartIndices {
  int point = 0;
  int curve = 0;
};

struct RealizeCurveTask {
  CurvesElementStartIndices start_indices;

  const RealizeCurveInfo *curve_info;
  /* Transformation applied to the position of control points and handles. */
  float4x4 transform;
  AttributeFallbacksArray attribute_fallbacks;
  /** Only used when the output contains an output attribute. */
  uint32_t id = 0;
};

struct AllPointCloudsInfo {
  /** Ordering of all attributes that are propagated to the output point cloud generically. */
  OrderedAttributes attributes;
  /** Ordering of the original point clouds that are joined. */
  VectorSet<const PointCloud *> order;
  /** Preprocessed data about every original point cloud. This is ordered by #order. */
  Array<PointCloudRealizeInfo> realize_info;
  bool create_id_attribute = false;
};

struct AllMeshesInfo {
  /** Ordering of all attributes that are propagated to the output mesh generically. */
  OrderedAttributes attributes;
  /** Ordering of the original meshes that are joined. */
  VectorSet<const Mesh *> order;
  /** Preprocessed data about every original mesh. This is ordered by #order. */
  Array<MeshRealizeInfo> realize_info;
  /** Ordered materials on the output mesh. */
  VectorSet<Material *> materials;
  bool create_id_attribute = false;
};

struct AllCurvesInfo {
  /** Ordering of all attributes that are propagated to the output curve generically. */
  OrderedAttributes attributes;
  /** Ordering of the original curves that are joined. */
  VectorSet<const Curves *> order;
  /** Preprocessed data about every original curve. This is ordered by #order. */
  Array<RealizeCurveInfo> realize_info;
  bool create_id_attribute = false;
  bool create_handle_postion_attributes = false;
  bool create_radius_attribute = false;
};

/** Collects all tasks that need to be executed to realize all instances. */
struct GatherTasks {
  Vector<RealizePointCloudTask> pointcloud_tasks;
  Vector<RealizeMeshTask> mesh_tasks;
  Vector<RealizeCurveTask> curve_tasks;

  /* Volumes only have very simple support currently. Only the first found volume is put into the
   * output. */
  UserCounter<VolumeComponent> first_volume;
};

/** Current offsets while during the gather operation. */
struct GatherOffsets {
  int pointcloud_offset = 0;
  MeshElementStartIndices mesh_offsets;
  CurvesElementStartIndices curves_offsets;
};

struct GatherTasksInfo {
  /** Static information about all geometries that are joined. */
  const AllPointCloudsInfo &pointclouds;
  const AllMeshesInfo &meshes;
  const AllCurvesInfo &curves;
  bool create_id_attribute_on_any_component = false;

  /**
   * Under some circumstances, temporary arrays need to be allocated during the gather operation.
   * For example, when an instance attribute has to be realized as a different data type. This
   * array owns all the temporary arrays so that they can live until all processing is done.
   * Use #std::unique_ptr to avoid depending on whether #GArray has an inline buffer or not.
   */
  Vector<std::unique_ptr<GArray<>>> &r_temporary_arrays;

  /** All gathered tasks. */
  GatherTasks r_tasks;
  /** Current offsets while gathering tasks. */
  GatherOffsets r_offsets;
};

/**
 * Information about the parent instances in the current context.
 */
struct InstanceContext {
  /** Ordered by #AllPointCloudsInfo.attributes. */
  AttributeFallbacksArray pointclouds;
  /** Ordered by #AllMeshesInfo.attributes. */
  AttributeFallbacksArray meshes;
  /** Ordered by #AllCurvesInfo.attributes. */
  AttributeFallbacksArray curves;
  /** Id mixed from all parent instances. */
  uint32_t id = 0;

  InstanceContext(const GatherTasksInfo &gather_info)
      : pointclouds(gather_info.pointclouds.attributes.size()),
        meshes(gather_info.meshes.attributes.size()),
        curves(gather_info.curves.attributes.size())
  {
  }
};

static void copy_transformed_positions(const Span<float3> src,
                                       const float4x4 &transform,
                                       MutableSpan<float3> dst)
{
  threading::parallel_for(src.index_range(), 1024, [&](const IndexRange range) {
    for (const int i : range) {
      dst[i] = transform * src[i];
    }
  });
}

static void threaded_copy(const GSpan src, GMutableSpan dst)
{
  BLI_assert(src.size() == dst.size());
  BLI_assert(src.type() == dst.type());
  threading::parallel_for(IndexRange(src.size()), 1024, [&](const IndexRange range) {
    src.type().copy_construct_n(src.slice(range).data(), dst.slice(range).data(), range.size());
  });
}

static void threaded_fill(const GPointer value, GMutableSpan dst)
{
  BLI_assert(*value.type() == dst.type());
  threading::parallel_for(IndexRange(dst.size()), 1024, [&](const IndexRange range) {
    value.type()->fill_construct_n(value.get(), dst.slice(range).data(), range.size());
  });
}

static void copy_generic_attributes_to_result(
    const Span<std::optional<GVArray_GSpan>> src_attributes,
    const AttributeFallbacksArray &attribute_fallbacks,
    const OrderedAttributes &ordered_attributes,
    const FunctionRef<IndexRange(AttributeDomain)> &range_fn,
    MutableSpan<GMutableSpan> dst_attributes)
{
  threading::parallel_for(dst_attributes.index_range(), 10, [&](const IndexRange attribute_range) {
    for (const int attribute_index : attribute_range) {
      const AttributeDomain domain = ordered_attributes.kinds[attribute_index].domain;
      const IndexRange element_slice = range_fn(domain);

      GMutableSpan dst_span = dst_attributes[attribute_index].slice(element_slice);
      if (src_attributes[attribute_index].has_value()) {
        threaded_copy(*src_attributes[attribute_index], dst_span);
      }
      else {
        const CPPType &cpp_type = dst_span.type();
        const void *fallback = attribute_fallbacks.array[attribute_index] == nullptr ?
                                   cpp_type.default_value() :
                                   attribute_fallbacks.array[attribute_index];
        threaded_fill({cpp_type, fallback}, dst_span);
      }
    }
  });
}

static void create_result_ids(const RealizeInstancesOptions &options,
                              Span<int> stored_ids,
                              const int task_id,
                              MutableSpan<int> dst_ids)
{
  if (options.keep_original_ids) {
    if (stored_ids.is_empty()) {
      dst_ids.fill(0);
    }
    else {
      dst_ids.copy_from(stored_ids);
    }
  }
  else {
    if (stored_ids.is_empty()) {
      threading::parallel_for(dst_ids.index_range(), 1024, [&](const IndexRange range) {
        for (const int i : range) {
          dst_ids[i] = noise::hash(task_id, i);
        }
      });
    }
    else {
      threading::parallel_for(dst_ids.index_range(), 1024, [&](const IndexRange range) {
        for (const int i : range) {
          dst_ids[i] = noise::hash(task_id, stored_ids[i]);
        }
      });
    }
  }
}

/* -------------------------------------------------------------------- */
/** \name Gather Realize Tasks
 * \{ */

/* Forward declaration. */
static void gather_realize_tasks_recursive(GatherTasksInfo &gather_info,
                                           const GeometrySet &geometry_set,
                                           const float4x4 &base_transform,
                                           const InstanceContext &base_instance_context);

/**
 * Checks which of the #ordered_attributes exist on the #instances_component. For each attribute
 * that exists on the instances, a pair is returned that contains the attribute index and the
 * corresponding attribute data.
 */
static Vector<std::pair<int, GSpan>> prepare_attribute_fallbacks(
    GatherTasksInfo &gather_info,
    const InstancesComponent &instances_component,
    const OrderedAttributes &ordered_attributes)
{
  Vector<std::pair<int, GSpan>> attributes_to_override;
  const CustomDataAttributes &attributes = instances_component.attributes();
  attributes.foreach_attribute(
      [&](const AttributeIDRef &attribute_id, const AttributeMetaData &meta_data) {
        const int attribute_index = ordered_attributes.ids.index_of_try(attribute_id);
        if (attribute_index == -1) {
          /* The attribute is not propagated to the final geometry. */
          return true;
        }
        GSpan span = *attributes.get_for_read(attribute_id);
        const CustomDataType expected_type = ordered_attributes.kinds[attribute_index].data_type;
        if (meta_data.data_type != expected_type) {
          const CPPType &from_type = span.type();
          const CPPType &to_type = *custom_data_type_to_cpp_type(expected_type);
          const bke::DataTypeConversions &conversions = bke::get_implicit_type_conversions();
          if (!conversions.is_convertible(from_type, to_type)) {
            /* Ignore the attribute because it can not be converted to the desired type. */
            return true;
          }
          /* Convert the attribute on the instances component to the expected attribute type. */
          std::unique_ptr<GArray<>> temporary_array = std::make_unique<GArray<>>(
              to_type, instances_component.instances_amount());
          conversions.convert_to_initialized_n(span, temporary_array->as_mutable_span());
          span = temporary_array->as_span();
          gather_info.r_temporary_arrays.append(std::move(temporary_array));
        }
        attributes_to_override.append({attribute_index, span});
        return true;
      },
      ATTR_DOMAIN_INSTANCE);
  return attributes_to_override;
}

/**
 * Calls #fn for every geometry in the given #InstanceReference. Also passes on the transformation
 * that is applied to every instance.
 */
static void foreach_geometry_in_reference(
    const InstanceReference &reference,
    const float4x4 &base_transform,
    const uint32_t id,
    FunctionRef<void(const GeometrySet &geometry_set, const float4x4 &transform, uint32_t id)> fn)
{
  switch (reference.type()) {
    case InstanceReference::Type::Object: {
      const Object &object = reference.object();
      const GeometrySet object_geometry_set = object_get_evaluated_geometry_set(object);
      fn(object_geometry_set, base_transform, id);
      break;
    }
    case InstanceReference::Type::Collection: {
      Collection &collection = reference.collection();
      float4x4 offset_matrix = float4x4::identity();
      sub_v3_v3(offset_matrix.values[3], collection.instance_offset);
      int index = 0;
      FOREACH_COLLECTION_OBJECT_RECURSIVE_BEGIN (&collection, object) {
        const GeometrySet object_geometry_set = object_get_evaluated_geometry_set(*object);
        const float4x4 matrix = base_transform * offset_matrix * object->obmat;
        const int sub_id = noise::hash(id, index);
        fn(object_geometry_set, matrix, sub_id);
        index++;
      }
      FOREACH_COLLECTION_OBJECT_RECURSIVE_END;
      break;
    }
    case InstanceReference::Type::GeometrySet: {
      const GeometrySet &instance_geometry_set = reference.geometry_set();
      fn(instance_geometry_set, base_transform, id);
      break;
    }
    case InstanceReference::Type::None: {
      break;
    }
  }
}

static void gather_realize_tasks_for_instances(GatherTasksInfo &gather_info,
                                               const InstancesComponent &instances_component,
                                               const float4x4 &base_transform,
                                               const InstanceContext &base_instance_context)
{
  const Span<InstanceReference> references = instances_component.references();
  const Span<int> handles = instances_component.instance_reference_handles();
  const Span<float4x4> transforms = instances_component.instance_transforms();

  Span<int> stored_instance_ids;
  if (gather_info.create_id_attribute_on_any_component) {
    std::optional<GSpan> ids = instances_component.attributes().get_for_read("id");
    if (ids.has_value()) {
      stored_instance_ids = ids->typed<int>();
    }
  }

  /* Prepare attribute fallbacks. */
  InstanceContext instance_context = base_instance_context;
  Vector<std::pair<int, GSpan>> pointcloud_attributes_to_override = prepare_attribute_fallbacks(
      gather_info, instances_component, gather_info.pointclouds.attributes);
  Vector<std::pair<int, GSpan>> mesh_attributes_to_override = prepare_attribute_fallbacks(
      gather_info, instances_component, gather_info.meshes.attributes);
  Vector<std::pair<int, GSpan>> curve_attributes_to_override = prepare_attribute_fallbacks(
      gather_info, instances_component, gather_info.curves.attributes);

  for (const int i : transforms.index_range()) {
    const int handle = handles[i];
    const float4x4 &transform = transforms[i];
    const InstanceReference &reference = references[handle];
    const float4x4 new_base_transform = base_transform * transform;

    /* Update attribute fallbacks for the current instance. */
    for (const std::pair<int, GSpan> &pair : pointcloud_attributes_to_override) {
      instance_context.pointclouds.array[pair.first] = pair.second[i];
    }
    for (const std::pair<int, GSpan> &pair : mesh_attributes_to_override) {
      instance_context.meshes.array[pair.first] = pair.second[i];
    }
    for (const std::pair<int, GSpan> &pair : curve_attributes_to_override) {
      instance_context.curves.array[pair.first] = pair.second[i];
    }

    uint32_t local_instance_id = 0;
    if (gather_info.create_id_attribute_on_any_component) {
      if (stored_instance_ids.is_empty()) {
        local_instance_id = (uint32_t)i;
      }
      else {
        local_instance_id = (uint32_t)stored_instance_ids[i];
      }
    }
    const uint32_t instance_id = noise::hash(base_instance_context.id, local_instance_id);

    /* Add realize tasks for all referenced geometry sets recursively. */
    foreach_geometry_in_reference(reference,
                                  new_base_transform,
                                  instance_id,
                                  [&](const GeometrySet &instance_geometry_set,
                                      const float4x4 &transform,
                                      const uint32_t id) {
                                    instance_context.id = id;
                                    gather_realize_tasks_recursive(gather_info,
                                                                   instance_geometry_set,
                                                                   transform,
                                                                   instance_context);
                                  });
  }
}

/**
 * Gather tasks for all geometries in the #geometry_set.
 */
static void gather_realize_tasks_recursive(GatherTasksInfo &gather_info,
                                           const GeometrySet &geometry_set,
                                           const float4x4 &base_transform,
                                           const InstanceContext &base_instance_context)
{
  for (const GeometryComponent *component : geometry_set.get_components_for_read()) {
    const GeometryComponentType type = component->type();
    switch (type) {
      case GEO_COMPONENT_TYPE_MESH: {
        const MeshComponent &mesh_component = *static_cast<const MeshComponent *>(component);
        const Mesh *mesh = mesh_component.get_for_read();
        if (mesh != nullptr && mesh->totvert > 0) {
          const int mesh_index = gather_info.meshes.order.index_of(mesh);
          const MeshRealizeInfo &mesh_info = gather_info.meshes.realize_info[mesh_index];
          gather_info.r_tasks.mesh_tasks.append({gather_info.r_offsets.mesh_offsets,
                                                 &mesh_info,
                                                 base_transform,
                                                 base_instance_context.meshes,
                                                 base_instance_context.id});
          gather_info.r_offsets.mesh_offsets.vertex += mesh->totvert;
          gather_info.r_offsets.mesh_offsets.edge += mesh->totedge;
          gather_info.r_offsets.mesh_offsets.loop += mesh->totloop;
          gather_info.r_offsets.mesh_offsets.poly += mesh->totpoly;
        }
        break;
      }
      case GEO_COMPONENT_TYPE_POINT_CLOUD: {
        const PointCloudComponent &pointcloud_component =
            *static_cast<const PointCloudComponent *>(component);
        const PointCloud *pointcloud = pointcloud_component.get_for_read();
        if (pointcloud != nullptr && pointcloud->totpoint > 0) {
          const int pointcloud_index = gather_info.pointclouds.order.index_of(pointcloud);
          const PointCloudRealizeInfo &pointcloud_info =
              gather_info.pointclouds.realize_info[pointcloud_index];
          gather_info.r_tasks.pointcloud_tasks.append({gather_info.r_offsets.pointcloud_offset,
                                                       &pointcloud_info,
                                                       base_transform,
                                                       base_instance_context.pointclouds,
                                                       base_instance_context.id});
          gather_info.r_offsets.pointcloud_offset += pointcloud->totpoint;
        }
        break;
      }
      case GEO_COMPONENT_TYPE_CURVE: {
        const CurveComponent &curve_component = *static_cast<const CurveComponent *>(component);
        const Curves *curves = curve_component.get_for_read();
        if (curves != nullptr && curves->geometry.curve_size > 0) {
          const int curve_index = gather_info.curves.order.index_of(curves);
          const RealizeCurveInfo &curve_info = gather_info.curves.realize_info[curve_index];
          gather_info.r_tasks.curve_tasks.append({gather_info.r_offsets.curves_offsets,
                                                  &curve_info,
                                                  base_transform,
                                                  base_instance_context.curves,
                                                  base_instance_context.id});
          gather_info.r_offsets.curves_offsets.point += curves->geometry.point_size;
          gather_info.r_offsets.curves_offsets.curve += curves->geometry.curve_size;
        }
        break;
      }
      case GEO_COMPONENT_TYPE_INSTANCES: {
        const InstancesComponent &instances_component = *static_cast<const InstancesComponent *>(
            component);
        gather_realize_tasks_for_instances(
            gather_info, instances_component, base_transform, base_instance_context);
        break;
      }
      case GEO_COMPONENT_TYPE_VOLUME: {
        const VolumeComponent *volume_component = static_cast<const VolumeComponent *>(component);
        if (!gather_info.r_tasks.first_volume) {
          volume_component->user_add();
          gather_info.r_tasks.first_volume = const_cast<VolumeComponent *>(volume_component);
        }
        break;
      }
    }
  }
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Point Cloud
 * \{ */

static OrderedAttributes gather_generic_pointcloud_attributes_to_propagate(
    const GeometrySet &in_geometry_set, const RealizeInstancesOptions &options, bool &r_create_id)
{
  Vector<GeometryComponentType> src_component_types;
  src_component_types.append(GEO_COMPONENT_TYPE_POINT_CLOUD);
  if (options.realize_instance_attributes) {
    src_component_types.append(GEO_COMPONENT_TYPE_INSTANCES);
  }

  Map<AttributeIDRef, AttributeKind> attributes_to_propagate;
  in_geometry_set.gather_attributes_for_propagation(
      src_component_types, GEO_COMPONENT_TYPE_POINT_CLOUD, true, attributes_to_propagate);
  attributes_to_propagate.remove("position");
  r_create_id = attributes_to_propagate.pop_try("id").has_value();
  OrderedAttributes ordered_attributes;
  for (const auto item : attributes_to_propagate.items()) {
    ordered_attributes.ids.add_new(item.key);
    ordered_attributes.kinds.append(item.value);
  }
  return ordered_attributes;
}

static void gather_pointclouds_to_realize(const GeometrySet &geometry_set,
                                          VectorSet<const PointCloud *> &r_pointclouds)
{
  if (const PointCloud *pointcloud = geometry_set.get_pointcloud_for_read()) {
    if (pointcloud->totpoint > 0) {
      r_pointclouds.add(pointcloud);
    }
  }
  if (const InstancesComponent *instances =
          geometry_set.get_component_for_read<InstancesComponent>()) {
    instances->foreach_referenced_geometry([&](const GeometrySet &instance_geometry_set) {
      gather_pointclouds_to_realize(instance_geometry_set, r_pointclouds);
    });
  }
}

static AllPointCloudsInfo preprocess_pointclouds(const GeometrySet &geometry_set,
                                                 const RealizeInstancesOptions &options)
{
  AllPointCloudsInfo info;
  info.attributes = gather_generic_pointcloud_attributes_to_propagate(
      geometry_set, options, info.create_id_attribute);

  gather_pointclouds_to_realize(geometry_set, info.order);
  info.realize_info.reinitialize(info.order.size());
  for (const int pointcloud_index : info.realize_info.index_range()) {
    PointCloudRealizeInfo &pointcloud_info = info.realize_info[pointcloud_index];
    const PointCloud *pointcloud = info.order[pointcloud_index];
    pointcloud_info.pointcloud = pointcloud;

    /* Access attributes. */
    PointCloudComponent component;
    component.replace(const_cast<PointCloud *>(pointcloud), GeometryOwnershipType::ReadOnly);
    pointcloud_info.attributes.reinitialize(info.attributes.size());
    for (const int attribute_index : info.attributes.index_range()) {
      const AttributeIDRef &attribute_id = info.attributes.ids[attribute_index];
      const CustomDataType data_type = info.attributes.kinds[attribute_index].data_type;
      const AttributeDomain domain = info.attributes.kinds[attribute_index].domain;
      if (component.attribute_exists(attribute_id)) {
        GVArray attribute = component.attribute_get_for_read(attribute_id, domain, data_type);
        pointcloud_info.attributes[attribute_index].emplace(std::move(attribute));
      }
    }
    if (info.create_id_attribute) {
      ReadAttributeLookup ids_lookup = component.attribute_try_get_for_read("id");
      if (ids_lookup) {
        pointcloud_info.stored_ids = ids_lookup.varray.get_internal_span().typed<int>();
      }
    }
  }
  return info;
}

static void execute_realize_pointcloud_task(const RealizeInstancesOptions &options,
                                            const RealizePointCloudTask &task,
                                            const OrderedAttributes &ordered_attributes,
                                            PointCloud &dst_pointcloud,
                                            MutableSpan<GMutableSpan> dst_attribute_spans,
                                            MutableSpan<int> all_dst_ids)
{
  const PointCloudRealizeInfo &pointcloud_info = *task.pointcloud_info;
  const PointCloud &pointcloud = *pointcloud_info.pointcloud;
  const Span<float3> src_positions{(float3 *)pointcloud.co, pointcloud.totpoint};
  const IndexRange point_slice{task.start_index, pointcloud.totpoint};
  MutableSpan<float3> dst_positions{(float3 *)dst_pointcloud.co + task.start_index,
                                    pointcloud.totpoint};

  copy_transformed_positions(src_positions, task.transform, dst_positions);

  /* Create point ids. */
  if (!all_dst_ids.is_empty()) {
    create_result_ids(
        options, pointcloud_info.stored_ids, task.id, all_dst_ids.slice(point_slice));
  }

  copy_generic_attributes_to_result(
      pointcloud_info.attributes,
      task.attribute_fallbacks,
      ordered_attributes,
      [&](const AttributeDomain domain) {
        BLI_assert(domain == ATTR_DOMAIN_POINT);
        UNUSED_VARS_NDEBUG(domain);
        return point_slice;
      },
      dst_attribute_spans);
}

static void execute_realize_pointcloud_tasks(const RealizeInstancesOptions &options,
                                             const AllPointCloudsInfo &all_pointclouds_info,
                                             const Span<RealizePointCloudTask> tasks,
                                             const OrderedAttributes &ordered_attributes,
                                             GeometrySet &r_realized_geometry)
{
  if (tasks.is_empty()) {
    return;
  }

  const RealizePointCloudTask &last_task = tasks.last();
  const PointCloud &last_pointcloud = *last_task.pointcloud_info->pointcloud;
  const int tot_points = last_task.start_index + last_pointcloud.totpoint;

  /* Allocate new point cloud. */
  PointCloud *dst_pointcloud = BKE_pointcloud_new_nomain(tot_points);
  PointCloudComponent &dst_component =
      r_realized_geometry.get_component_for_write<PointCloudComponent>();
  dst_component.replace(dst_pointcloud);

  /* Prepare id attribute. */
  OutputAttribute_Typed<int> point_ids;
  MutableSpan<int> point_ids_span;
  if (all_pointclouds_info.create_id_attribute) {
    point_ids = dst_component.attribute_try_get_for_output_only<int>("id", ATTR_DOMAIN_POINT);
    point_ids_span = point_ids.as_span();
  }

  /* Prepare generic output attributes. */
  Vector<OutputAttribute> dst_attributes;
  Vector<GMutableSpan> dst_attribute_spans;
  for (const int attribute_index : ordered_attributes.index_range()) {
    const AttributeIDRef &attribute_id = ordered_attributes.ids[attribute_index];
    const CustomDataType data_type = ordered_attributes.kinds[attribute_index].data_type;
    OutputAttribute dst_attribute = dst_component.attribute_try_get_for_output_only(
        attribute_id, ATTR_DOMAIN_POINT, data_type);
    dst_attribute_spans.append(dst_attribute.as_span());
    dst_attributes.append(std::move(dst_attribute));
  }

  /* Actually execute all tasks. */
  threading::parallel_for(tasks.index_range(), 100, [&](const IndexRange task_range) {
    for (const int task_index : task_range) {
      const RealizePointCloudTask &task = tasks[task_index];
      execute_realize_pointcloud_task(
          options, task, ordered_attributes, *dst_pointcloud, dst_attribute_spans, point_ids_span);
    }
  });

  /* Save modified attributes. */
  for (OutputAttribute &dst_attribute : dst_attributes) {
    dst_attribute.save();
  }
  if (point_ids) {
    point_ids.save();
  }
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Mesh
 * \{ */

static OrderedAttributes gather_generic_mesh_attributes_to_propagate(
    const GeometrySet &in_geometry_set, const RealizeInstancesOptions &options, bool &r_create_id)
{
  Vector<GeometryComponentType> src_component_types;
  src_component_types.append(GEO_COMPONENT_TYPE_MESH);
  if (options.realize_instance_attributes) {
    src_component_types.append(GEO_COMPONENT_TYPE_INSTANCES);
  }

  Map<AttributeIDRef, AttributeKind> attributes_to_propagate;
  in_geometry_set.gather_attributes_for_propagation(
      src_component_types, GEO_COMPONENT_TYPE_MESH, true, attributes_to_propagate);
  attributes_to_propagate.remove("position");
  attributes_to_propagate.remove("normal");
  attributes_to_propagate.remove("material_index");
  attributes_to_propagate.remove("shade_smooth");
  attributes_to_propagate.remove("crease");
  r_create_id = attributes_to_propagate.pop_try("id").has_value();
  OrderedAttributes ordered_attributes;
  for (const auto item : attributes_to_propagate.items()) {
    ordered_attributes.ids.add_new(item.key);
    ordered_attributes.kinds.append(item.value);
  }
  return ordered_attributes;
}

static void gather_meshes_to_realize(const GeometrySet &geometry_set,
                                     VectorSet<const Mesh *> &r_meshes)
{
  if (const Mesh *mesh = geometry_set.get_mesh_for_read()) {
    if (mesh->totvert > 0) {
      r_meshes.add(mesh);
    }
  }
  if (const InstancesComponent *instances =
          geometry_set.get_component_for_read<InstancesComponent>()) {
    instances->foreach_referenced_geometry([&](const GeometrySet &instance_geometry_set) {
      gather_meshes_to_realize(instance_geometry_set, r_meshes);
    });
  }
}

static AllMeshesInfo preprocess_meshes(const GeometrySet &geometry_set,
                                       const RealizeInstancesOptions &options)
{
  AllMeshesInfo info;
  info.attributes = gather_generic_mesh_attributes_to_propagate(
      geometry_set, options, info.create_id_attribute);

  gather_meshes_to_realize(geometry_set, info.order);
  for (const Mesh *mesh : info.order) {
    for (const int slot_index : IndexRange(mesh->totcol)) {
      Material *material = mesh->mat[slot_index];
      info.materials.add(material);
    }
  }
  info.realize_info.reinitialize(info.order.size());
  for (const int mesh_index : info.realize_info.index_range()) {
    MeshRealizeInfo &mesh_info = info.realize_info[mesh_index];
    const Mesh *mesh = info.order[mesh_index];
    mesh_info.mesh = mesh;

    /* Create material index mapping. */
    mesh_info.material_index_map.reinitialize(mesh->totcol);
    for (const int old_slot_index : IndexRange(mesh->totcol)) {
      Material *material = mesh->mat[old_slot_index];
      const int new_slot_index = info.materials.index_of(material);
      mesh_info.material_index_map[old_slot_index] = new_slot_index;
    }

    /* Access attributes. */
    MeshComponent component;
    component.replace(const_cast<Mesh *>(mesh), GeometryOwnershipType::ReadOnly);
    mesh_info.attributes.reinitialize(info.attributes.size());
    for (const int attribute_index : info.attributes.index_range()) {
      const AttributeIDRef &attribute_id = info.attributes.ids[attribute_index];
      const CustomDataType data_type = info.attributes.kinds[attribute_index].data_type;
      const AttributeDomain domain = info.attributes.kinds[attribute_index].domain;
      if (component.attribute_exists(attribute_id)) {
        GVArray attribute = component.attribute_get_for_read(attribute_id, domain, data_type);
        mesh_info.attributes[attribute_index].emplace(std::move(attribute));
      }
    }
    if (info.create_id_attribute) {
      ReadAttributeLookup ids_lookup = component.attribute_try_get_for_read("id");
      if (ids_lookup) {
        mesh_info.stored_vertex_ids = ids_lookup.varray.get_internal_span().typed<int>();
      }
    }
  }
  return info;
}

static void execute_realize_mesh_task(const RealizeInstancesOptions &options,
                                      const RealizeMeshTask &task,
                                      const OrderedAttributes &ordered_attributes,
                                      Mesh &dst_mesh,
                                      MutableSpan<GMutableSpan> dst_attribute_spans,
                                      MutableSpan<int> all_dst_vertex_ids)
{
  const MeshRealizeInfo &mesh_info = *task.mesh_info;
  const Mesh &mesh = *mesh_info.mesh;

  const Span<MVert> src_verts{mesh.mvert, mesh.totvert};
  const Span<MEdge> src_edges{mesh.medge, mesh.totedge};
  const Span<MLoop> src_loops{mesh.mloop, mesh.totloop};
  const Span<MPoly> src_polys{mesh.mpoly, mesh.totpoly};

  MutableSpan<MVert> dst_verts{dst_mesh.mvert + task.start_indices.vertex, mesh.totvert};
  MutableSpan<MEdge> dst_edges{dst_mesh.medge + task.start_indices.edge, mesh.totedge};
  MutableSpan<MLoop> dst_loops{dst_mesh.mloop + task.start_indices.loop, mesh.totloop};
  MutableSpan<MPoly> dst_polys{dst_mesh.mpoly + task.start_indices.poly, mesh.totpoly};

  const Span<int> material_index_map = mesh_info.material_index_map;

  threading::parallel_for(IndexRange(mesh.totvert), 1024, [&](const IndexRange vert_range) {
    for (const int i : vert_range) {
      const MVert &src_vert = src_verts[i];
      MVert &dst_vert = dst_verts[i];
      dst_vert = src_vert;
      copy_v3_v3(dst_vert.co, task.transform * float3(src_vert.co));
    }
  });
  threading::parallel_for(IndexRange(mesh.totedge), 1024, [&](const IndexRange edge_range) {
    for (const int i : edge_range) {
      const MEdge &src_edge = src_edges[i];
      MEdge &dst_edge = dst_edges[i];
      dst_edge = src_edge;
      dst_edge.v1 += task.start_indices.vertex;
      dst_edge.v2 += task.start_indices.vertex;
    }
  });
  threading::parallel_for(IndexRange(mesh.totloop), 1024, [&](const IndexRange loop_range) {
    for (const int i : loop_range) {
      const MLoop &src_loop = src_loops[i];
      MLoop &dst_loop = dst_loops[i];
      dst_loop = src_loop;
      dst_loop.v += task.start_indices.vertex;
      dst_loop.e += task.start_indices.edge;
    }
  });
  threading::parallel_for(IndexRange(mesh.totpoly), 1024, [&](const IndexRange poly_range) {
    for (const int i : poly_range) {
      const MPoly &src_poly = src_polys[i];
      MPoly &dst_poly = dst_polys[i];
      dst_poly = src_poly;
      dst_poly.loopstart += task.start_indices.loop;
      if (src_poly.mat_nr >= 0 && src_poly.mat_nr < mesh.totcol) {
        dst_poly.mat_nr = material_index_map[src_poly.mat_nr];
      }
      else {
        /* The material index was invalid before. */
        dst_poly.mat_nr = 0;
      }
    }
  });

  if (!all_dst_vertex_ids.is_empty()) {
    create_result_ids(options,
                      mesh_info.stored_vertex_ids,
                      task.id,
                      all_dst_vertex_ids.slice(task.start_indices.vertex, mesh.totvert));
  }

  copy_generic_attributes_to_result(
      mesh_info.attributes,
      task.attribute_fallbacks,
      ordered_attributes,
      [&](const AttributeDomain domain) {
        switch (domain) {
          case ATTR_DOMAIN_POINT:
            return IndexRange(task.start_indices.vertex, mesh.totvert);
          case ATTR_DOMAIN_EDGE:
            return IndexRange(task.start_indices.edge, mesh.totedge);
          case ATTR_DOMAIN_CORNER:
            return IndexRange(task.start_indices.loop, mesh.totloop);
          case ATTR_DOMAIN_FACE:
            return IndexRange(task.start_indices.poly, mesh.totpoly);
          default:
            BLI_assert_unreachable();
            return IndexRange();
        }
      },
      dst_attribute_spans);
}

static void execute_realize_mesh_tasks(const RealizeInstancesOptions &options,
                                       const AllMeshesInfo &all_meshes_info,
                                       const Span<RealizeMeshTask> tasks,
                                       const OrderedAttributes &ordered_attributes,
                                       const VectorSet<Material *> &ordered_materials,
                                       GeometrySet &r_realized_geometry)
{
  if (tasks.is_empty()) {
    return;
  }

  const RealizeMeshTask &last_task = tasks.last();
  const Mesh &last_mesh = *last_task.mesh_info->mesh;
  const int tot_vertices = last_task.start_indices.vertex + last_mesh.totvert;
  const int tot_edges = last_task.start_indices.edge + last_mesh.totedge;
  const int tot_loops = last_task.start_indices.loop + last_mesh.totloop;
  const int tot_poly = last_task.start_indices.poly + last_mesh.totpoly;

  Mesh *dst_mesh = BKE_mesh_new_nomain(tot_vertices, tot_edges, 0, tot_loops, tot_poly);
  MeshComponent &dst_component = r_realized_geometry.get_component_for_write<MeshComponent>();
  dst_component.replace(dst_mesh);

  /* Copy settings from the first input geometry set with a mesh. */
  const RealizeMeshTask &first_task = tasks.first();
  const Mesh &first_mesh = *first_task.mesh_info->mesh;
  BKE_mesh_copy_parameters_for_eval(dst_mesh, &first_mesh);

  /* Add materials. */
  for (const int i : IndexRange(ordered_materials.size())) {
    Material *material = ordered_materials[i];
    BKE_id_material_eval_assign(&dst_mesh->id, i + 1, material);
  }

  /* Prepare id attribute. */
  OutputAttribute_Typed<int> vertex_ids;
  MutableSpan<int> vertex_ids_span;
  if (all_meshes_info.create_id_attribute) {
    vertex_ids = dst_component.attribute_try_get_for_output_only<int>("id", ATTR_DOMAIN_POINT);
    vertex_ids_span = vertex_ids.as_span();
  }

  /* Prepare generic output attributes. */
  Vector<OutputAttribute> dst_attributes;
  Vector<GMutableSpan> dst_attribute_spans;
  for (const int attribute_index : ordered_attributes.index_range()) {
    const AttributeIDRef &attribute_id = ordered_attributes.ids[attribute_index];
    const AttributeDomain domain = ordered_attributes.kinds[attribute_index].domain;
    const CustomDataType data_type = ordered_attributes.kinds[attribute_index].data_type;
    OutputAttribute dst_attribute = dst_component.attribute_try_get_for_output_only(
        attribute_id, domain, data_type);
    dst_attribute_spans.append(dst_attribute.as_span());
    dst_attributes.append(std::move(dst_attribute));
  }

  /* Actually execute all tasks. */
  threading::parallel_for(tasks.index_range(), 100, [&](const IndexRange task_range) {
    for (const int task_index : task_range) {
      const RealizeMeshTask &task = tasks[task_index];
      execute_realize_mesh_task(
          options, task, ordered_attributes, *dst_mesh, dst_attribute_spans, vertex_ids_span);
    }
  });

  /* Save modified attributes. */
  for (OutputAttribute &dst_attribute : dst_attributes) {
    dst_attribute.save();
  }
  if (vertex_ids) {
    vertex_ids.save();
  }

  BKE_mesh_normals_tag_dirty(dst_mesh);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Curves
 * \{ */

static OrderedAttributes gather_generic_curve_attributes_to_propagate(
    const GeometrySet &in_geometry_set, const RealizeInstancesOptions &options, bool &r_create_id)
{
  Vector<GeometryComponentType> src_component_types;
  src_component_types.append(GEO_COMPONENT_TYPE_CURVE);
  if (options.realize_instance_attributes) {
    src_component_types.append(GEO_COMPONENT_TYPE_INSTANCES);
  }

  Map<AttributeIDRef, AttributeKind> attributes_to_propagate;
  in_geometry_set.gather_attributes_for_propagation(
      src_component_types, GEO_COMPONENT_TYPE_CURVE, true, attributes_to_propagate);
  attributes_to_propagate.remove("position");
  attributes_to_propagate.remove("radius");
  attributes_to_propagate.remove("handle_right");
  attributes_to_propagate.remove("handle_left");
  r_create_id = attributes_to_propagate.pop_try("id").has_value();
  OrderedAttributes ordered_attributes;
  for (const auto item : attributes_to_propagate.items()) {
    ordered_attributes.ids.add_new(item.key);
    ordered_attributes.kinds.append(item.value);
  }
  return ordered_attributes;
}

static void gather_curves_to_realize(const GeometrySet &geometry_set,
                                     VectorSet<const Curves *> &r_curves)
{
  if (const Curves *curves = geometry_set.get_curves_for_read()) {
    if (curves->geometry.curve_size != 0) {
      r_curves.add(curves);
    }
  }
  if (const InstancesComponent *instances =
          geometry_set.get_component_for_read<InstancesComponent>()) {
    instances->foreach_referenced_geometry([&](const GeometrySet &instance_geometry_set) {
      gather_curves_to_realize(instance_geometry_set, r_curves);
    });
  }
}

static AllCurvesInfo preprocess_curves(const GeometrySet &geometry_set,
                                       const RealizeInstancesOptions &options)
{
  AllCurvesInfo info;
  info.attributes = gather_generic_curve_attributes_to_propagate(
      geometry_set, options, info.create_id_attribute);

  gather_curves_to_realize(geometry_set, info.order);
  info.realize_info.reinitialize(info.order.size());
  for (const int curve_index : info.realize_info.index_range()) {
    RealizeCurveInfo &curve_info = info.realize_info[curve_index];
    const Curves *curves = info.order[curve_index];
    curve_info.curves = curves;

    /* Access attributes. */
    CurveComponent component;
    component.replace(const_cast<Curves *>(curves), GeometryOwnershipType::ReadOnly);
    curve_info.attributes.reinitialize(info.attributes.size());
    for (const int attribute_index : info.attributes.index_range()) {
      const AttributeDomain domain = info.attributes.kinds[attribute_index].domain;
      const AttributeIDRef &attribute_id = info.attributes.ids[attribute_index];
      const CustomDataType data_type = info.attributes.kinds[attribute_index].data_type;
      if (component.attribute_exists(attribute_id)) {
        GVArray attribute = component.attribute_get_for_read(attribute_id, domain, data_type);
        curve_info.attributes[attribute_index].emplace(std::move(attribute));
      }
    }
    if (info.create_id_attribute) {
      ReadAttributeLookup ids_lookup = component.attribute_try_get_for_read("id");
      if (ids_lookup) {
        curve_info.stored_ids = ids_lookup.varray.get_internal_span().typed<int>();
      }
    }

    /* Retrieve the radius attribute, if it exists. */
    if (component.attribute_exists("radius")) {
      curve_info.radius = component
                              .attribute_get_for_read<float>("radius", ATTR_DOMAIN_POINT, 0.0f)
                              .get_internal_span();
      info.create_radius_attribute = true;
    }

    /* Retrieve handle position attributes, if they exist. */
    if (component.attribute_exists("handle_right")) {
      curve_info.handle_left = component
                                   .attribute_get_for_read<float3>(
                                       "handle_left", ATTR_DOMAIN_POINT, float3(0))
                                   .get_internal_span();
      curve_info.handle_right = component
                                    .attribute_get_for_read<float3>(
                                        "handle_right", ATTR_DOMAIN_POINT, float3(0))
                                    .get_internal_span();
      info.create_handle_postion_attributes = true;
    }
  }
  return info;
}

static void execute_realize_curve_task(const RealizeInstancesOptions &options,
                                       const AllCurvesInfo &all_curves_info,
                                       const RealizeCurveTask &task,
                                       const OrderedAttributes &ordered_attributes,
                                       bke::CurvesGeometry &dst_curves,
                                       MutableSpan<GMutableSpan> dst_attribute_spans,
                                       MutableSpan<int> all_dst_ids,
                                       MutableSpan<float3> all_handle_left,
                                       MutableSpan<float3> all_handle_right,
                                       MutableSpan<float> all_radii)
{
  const RealizeCurveInfo &curves_info = *task.curve_info;
  const Curves &curves_id = *curves_info.curves;
  const bke::CurvesGeometry &curves = bke::CurvesGeometry::wrap(curves_id.geometry);

  const IndexRange dst_point_range{task.start_indices.point, curves.points_num()};
  const IndexRange dst_curve_range{task.start_indices.curve, curves.curves_num()};

  copy_transformed_positions(
      curves.positions(), task.transform, dst_curves.positions().slice(dst_point_range));

  /* Copy and transform handle positions if necessary. */
  if (all_curves_info.create_handle_postion_attributes) {
    if (curves_info.handle_left.is_empty()) {
      all_handle_left.slice(dst_point_range).fill(float3(0));
    }
    else {
      copy_transformed_positions(
          curves_info.handle_left, task.transform, all_handle_left.slice(dst_point_range));
    }
    if (curves_info.handle_right.is_empty()) {
      all_handle_right.slice(dst_point_range).fill(float3(0));
    }
    else {
      copy_transformed_positions(
          curves_info.handle_right, task.transform, all_handle_right.slice(dst_point_range));
    }
  }

  /* Copy radius attribute with 1.0 default if it doesn't exist. */
  if (all_curves_info.create_radius_attribute) {
    if (curves_info.radius.is_empty()) {
      all_radii.slice(dst_point_range).fill(1.0f);
    }
    else {
      all_radii.slice(dst_point_range).copy_from(curves_info.radius);
    }
  }

  /* Copy curve offsets. */
  const Span<int> src_offsets = curves.offsets();
  const MutableSpan<int> dst_offsets = dst_curves.offsets().slice(dst_curve_range);
  threading::parallel_for(curves.curves_range(), 2048, [&](const IndexRange range) {
    for (const int i : range) {
      dst_offsets[i] = task.start_indices.point + src_offsets[i];
    }
  });

  if (!all_dst_ids.is_empty()) {
    create_result_ids(
        options, curves_info.stored_ids, task.id, all_dst_ids.slice(dst_point_range));
  }

  copy_generic_attributes_to_result(
      curves_info.attributes,
      task.attribute_fallbacks,
      ordered_attributes,
      [&](const AttributeDomain domain) {
        switch (domain) {
          case ATTR_DOMAIN_POINT:
            return IndexRange(task.start_indices.point, curves.points_num());
          case ATTR_DOMAIN_CURVE:
            return IndexRange(task.start_indices.curve, curves.curves_num());
          default:
            BLI_assert_unreachable();
            return IndexRange();
        }
      },
      dst_attribute_spans);
}

static void execute_realize_curve_tasks(const RealizeInstancesOptions &options,
                                        const AllCurvesInfo &all_curves_info,
                                        const Span<RealizeCurveTask> tasks,
                                        const OrderedAttributes &ordered_attributes,
                                        GeometrySet &r_realized_geometry)
{
  if (tasks.is_empty()) {
    return;
  }

  const RealizeCurveTask &last_task = tasks.last();
  const Curves &last_curves = *last_task.curve_info->curves;
  const int points_size = last_task.start_indices.point + last_curves.geometry.point_size;
  const int curves_size = last_task.start_indices.curve + last_curves.geometry.curve_size;

  /* Allocate new curves data-block. */
  Curves *dst_curves_id = bke::curves_new_nomain(points_size, curves_size);
  bke::CurvesGeometry &dst_curves = bke::CurvesGeometry::wrap(dst_curves_id->geometry);
  dst_curves.offsets().last() = points_size;
  CurveComponent &dst_component = r_realized_geometry.get_component_for_write<CurveComponent>();
  dst_component.replace(dst_curves_id);

  /* Prepare id attribute. */
  OutputAttribute_Typed<int> point_ids;
  MutableSpan<int> point_ids_span;
  if (all_curves_info.create_id_attribute) {
    point_ids = dst_component.attribute_try_get_for_output_only<int>("id", ATTR_DOMAIN_POINT);
    point_ids_span = point_ids.as_span();
  }

  /* Prepare generic output attributes. */
  Vector<OutputAttribute> dst_attributes;
  Vector<GMutableSpan> dst_attribute_spans;
  for (const int attribute_index : ordered_attributes.index_range()) {
    const AttributeIDRef &attribute_id = ordered_attributes.ids[attribute_index];
    const AttributeDomain domain = ordered_attributes.kinds[attribute_index].domain;
    const CustomDataType data_type = ordered_attributes.kinds[attribute_index].data_type;
    OutputAttribute dst_attribute = dst_component.attribute_try_get_for_output_only(
        attribute_id, domain, data_type);
    dst_attribute_spans.append(dst_attribute.as_span());
    dst_attributes.append(std::move(dst_attribute));
  }

  /* Prepare handle position attributes if necessary. */
  OutputAttribute_Typed<float3> handle_left;
  OutputAttribute_Typed<float3> handle_right;
  MutableSpan<float3> handle_left_span;
  MutableSpan<float3> handle_right_span;
  if (all_curves_info.create_handle_postion_attributes) {
    handle_left = dst_component.attribute_try_get_for_output_only<float3>("handle_left",
                                                                          ATTR_DOMAIN_POINT);
    handle_right = dst_component.attribute_try_get_for_output_only<float3>("handle_right",
                                                                           ATTR_DOMAIN_POINT);
    handle_left_span = handle_left.as_span();
    handle_right_span = handle_right.as_span();
  }

  /* Prepare radius attribute if necessary. */
  OutputAttribute_Typed<float> radius;
  MutableSpan<float> radius_span;
  if (all_curves_info.create_radius_attribute) {
    radius = dst_component.attribute_try_get_for_output_only<float>("radius", ATTR_DOMAIN_POINT);
    radius_span = radius.as_span();
  }

  /* Actually execute all tasks. */
  threading::parallel_for(tasks.index_range(), 100, [&](const IndexRange task_range) {
    for (const int task_index : task_range) {
      const RealizeCurveTask &task = tasks[task_index];
      execute_realize_curve_task(options,
                                 all_curves_info,
                                 task,
                                 ordered_attributes,
                                 dst_curves,
                                 dst_attribute_spans,
                                 point_ids_span,
                                 handle_left_span,
                                 handle_right_span,
                                 radius_span);
    }
  });

  /* Save modified attributes. */
  for (OutputAttribute &dst_attribute : dst_attributes) {
    dst_attribute.save();
  }
  if (point_ids) {
    point_ids.save();
  }
  if (radius) {
    radius.save();
  }
  if (all_curves_info.create_handle_postion_attributes) {
    handle_left.save();
    handle_right.save();
  }
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Realize Instances
 * \{ */

static void remove_id_attribute_from_instances(GeometrySet &geometry_set)
{
  geometry_set.modify_geometry_sets([&](GeometrySet &sub_geometry) {
    if (sub_geometry.has<InstancesComponent>()) {
      InstancesComponent &component = sub_geometry.get_component_for_write<InstancesComponent>();
      component.attributes().remove("id");
    }
  });
}

GeometrySet realize_instances(GeometrySet geometry_set, const RealizeInstancesOptions &options)
{
  /* The algorithm works in three steps:
   * 1. Preprocess each unique geometry that is instanced (e.g. each `Mesh`).
   * 2. Gather "tasks" that need to be executed to realize the instances. Each task corresponds to
   *    instances of the previously preprocessed geometry.
   * 3. Execute all tasks in parallel.
   */

  if (!geometry_set.has_instances()) {
    return geometry_set;
  }

  if (options.keep_original_ids) {
    remove_id_attribute_from_instances(geometry_set);
  }

  AllPointCloudsInfo all_pointclouds_info = preprocess_pointclouds(geometry_set, options);
  AllMeshesInfo all_meshes_info = preprocess_meshes(geometry_set, options);
  AllCurvesInfo all_curves_info = preprocess_curves(geometry_set, options);

  Vector<std::unique_ptr<GArray<>>> temporary_arrays;
  const bool create_id_attribute = all_pointclouds_info.create_id_attribute ||
                                   all_meshes_info.create_id_attribute ||
                                   all_curves_info.create_id_attribute;
  GatherTasksInfo gather_info = {all_pointclouds_info,
                                 all_meshes_info,
                                 all_curves_info,
                                 create_id_attribute,
                                 temporary_arrays};
  const float4x4 transform = float4x4::identity();
  InstanceContext attribute_fallbacks(gather_info);
  gather_realize_tasks_recursive(gather_info, geometry_set, transform, attribute_fallbacks);

  GeometrySet new_geometry_set;
  execute_realize_pointcloud_tasks(options,
                                   all_pointclouds_info,
                                   gather_info.r_tasks.pointcloud_tasks,
                                   all_pointclouds_info.attributes,
                                   new_geometry_set);
  execute_realize_mesh_tasks(options,
                             all_meshes_info,
                             gather_info.r_tasks.mesh_tasks,
                             all_meshes_info.attributes,
                             all_meshes_info.materials,
                             new_geometry_set);
  execute_realize_curve_tasks(options,
                              all_curves_info,
                              gather_info.r_tasks.curve_tasks,
                              all_curves_info.attributes,
                              new_geometry_set);

  if (gather_info.r_tasks.first_volume) {
    new_geometry_set.add(*gather_info.r_tasks.first_volume);
  }

  return new_geometry_set;
}

GeometrySet realize_instances_legacy(GeometrySet geometry_set)
{
  RealizeInstancesOptions options;
  options.keep_original_ids = true;
  options.realize_instance_attributes = false;
  return realize_instances(std::move(geometry_set), options);
}

/** \} */

}  // namespace blender::geometry
