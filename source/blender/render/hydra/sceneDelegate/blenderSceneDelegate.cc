/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <pxr/imaging/hd/light.h>
#include <pxr/imaging/hd/material.h>
#include <pxr/usd/usdLux/tokens.h>
#include <pxr/imaging/hdSt/tokens.h>
//#include <pxr/base/vt/array.h>

#include "glog/logging.h"

#include "blenderSceneDelegate.h"
#include "object.h"

namespace blender::render::hydra {

BlenderSceneDelegate::BlenderSceneDelegate(HdRenderIndex* parentIndex, SdfPath const& delegateID, BL::Depsgraph &b_depsgraph)
  : HdSceneDelegate(parentIndex, delegateID)
  , b_depsgraph(b_depsgraph)
  , is_populated(false)
{
}

void BlenderSceneDelegate::set_material(ObjectData &obj_data)
{
  Material *material = obj_data.material();
  if (!material) {
    obj_data.set_material_id(SdfPath::EmptyPath());
    return;
  }
  SdfPath mat_id = material_id(material);
  if (!material_data(mat_id)) {
    MaterialData mat_data(material);
    GetRenderIndex().InsertSprim(HdPrimTypeTokens->material, this, mat_id);
    mat_data.export_mtlx();
    materials[mat_id] = mat_data;
    LOG(INFO) << "Add material: " << mat_id << ", mtlx=" << mat_data.mtlx_path.GetResolvedPath();
  }
  obj_data.set_material_id(mat_id);
}

void BlenderSceneDelegate::update_material(Material *material)
{
  SdfPath mat_id = material_id(material);
  MaterialData *mat_data = material_data(mat_id);
  if (mat_data) {
    mat_data->export_mtlx();
    LOG(INFO) << "Update material: " << mat_id << ", mtlx=" << mat_data->mtlx_path.GetResolvedPath();
    GetRenderIndex().GetChangeTracker().MarkSprimDirty(mat_id, HdMaterial::AllDirty);
  }
}

void BlenderSceneDelegate::update_collection()
{
  HdRenderIndex &index = GetRenderIndex();

  /* add new objects */
  std::set<SdfPath> available_objects;
  for (auto &inst : b_depsgraph.object_instances) {
    if (inst.is_instance()) {
      continue;
    }
    Object *object = (Object *)inst.object().ptr.data;
    if (!supported_object(object)) {
      continue;
    }
    SdfPath obj_id = object_id(object);
    available_objects.insert(obj_id);

    if (!is_populated) {
      add_update_object(object, true, true, true);
    }
  }

  if (!is_populated) {
    return;
  }

  /* remove unused objects */
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    if (available_objects.find(it->first) != available_objects.end()) {
      continue;
    }
    LOG(INFO) << "Remove: " << it->first;
    if (index.GetRprim(it->first)) {
      index.RemoveRprim(it->first);
    }
    else {
      index.RemoveSprim(it->second.prim_type(), it->first);
    }
    objects.erase(it);
    it = objects.begin();
  }

  /* remove unused materials */
  std::set<SdfPath> available_materials;
  for (auto &obj : objects) {
    if (obj.second.has_data(TfToken("materialId"))) {
      available_materials.insert(obj.second.get_data(TfToken("materialId")).Get<SdfPath>());
    }
  }
  for (auto it = materials.begin(); it != materials.end(); ++it) {
    if (available_materials.find(it->first) != available_materials.end()) {
      continue;
    }
    LOG(INFO) << "Remove material: " << it->first;
    index.RemoveSprim(HdPrimTypeTokens->material, it->first);
    materials.erase(it);
    it = materials.begin();
  }
}

void BlenderSceneDelegate::add_update_object(Object *object, bool geometry, bool transform, bool shading)
{
  HdRenderIndex &index = GetRenderIndex();

  SdfPath obj_id = object_id(object);
  ObjectData *obj_data = object_data(obj_id);
  if (!obj_data) {
    ObjectData new_obj_data(object);
    if (new_obj_data.prim_type() == HdPrimTypeTokens->mesh) {
      LOG(INFO) << "Add mesh object: " << new_obj_data.name() << " id=" << obj_id;
      index.InsertRprim(new_obj_data.prim_type(), this, obj_id);
      set_material(new_obj_data);
    }
    else if (new_obj_data.type() == OB_LAMP) {
      LOG(INFO) << "Add light object: " << new_obj_data.name() << " id=" << obj_id;
      index.InsertSprim(new_obj_data.prim_type(), this, obj_id);
    }
    objects[obj_id] = new_obj_data;
    return;
  }

  if (geometry) {
    LOG(INFO) << "Full updated: " << obj_id;
    ObjectData new_obj_data(object);
    if (new_obj_data.prim_type() == HdPrimTypeTokens->mesh) {
      set_material(new_obj_data);
      index.GetChangeTracker().MarkRprimDirty(obj_id, HdChangeTracker::AllDirty);
    }
    else if (new_obj_data.type() == OB_LAMP) {
      index.GetChangeTracker().MarkSprimDirty(obj_id, HdLight::AllDirty);
    }
    objects[obj_id] = new_obj_data;
    return;
  }

  if (transform) {
    LOG(INFO) << "Transform updated: " << obj_id;
    if (obj_data->prim_type() == HdPrimTypeTokens->mesh) {
      index.GetChangeTracker().MarkRprimDirty(obj_id, HdChangeTracker::DirtyTransform);
    }
    else if (obj_data->type() == OB_LAMP) {
      index.GetChangeTracker().MarkSprimDirty(obj_id, HdLight::DirtyTransform);
    }
  }

  if (shading) {
    LOG(INFO) << "Shading updated: " << obj_id;
    if (obj_data->prim_type() == HdPrimTypeTokens->mesh) {
      index.GetChangeTracker().MarkRprimDirty(obj_id, HdChangeTracker::DirtyMaterialId);
    }
  }
}

ObjectData *BlenderSceneDelegate::object_data(SdfPath const &id)
{
  auto it = objects.find(id);
  if (it == objects.end()) {
    return nullptr;
  }
  return &it->second;
}

MaterialData *BlenderSceneDelegate::material_data(SdfPath const &id)
{
  auto it = materials.find(id);
  if (it == materials.end()) {
    return nullptr;
  }
  return &it->second;
}

SdfPath BlenderSceneDelegate::object_id(Object *object)
{
  char str[32];
  snprintf(str, 32, "O_%016llX", (uint64_t)object);
  return GetDelegateID().AppendElementString(str);
}

SdfPath BlenderSceneDelegate::material_id(Material *material)
{
  char str[32];
  snprintf(str, 32, "M_%016llX", (uint64_t)material);
  return GetDelegateID().AppendElementString(str);
}

bool BlenderSceneDelegate::supported_object(Object *object)
{
  return object->type == OB_MESH || object->type == OB_LAMP;
}

void BlenderSceneDelegate::Populate()
{
  LOG(INFO) << "Populate " << is_populated;

  if (is_populated) {
    for (auto &update : b_depsgraph.updates) {
      BL::ID id = update.id();
      LOG(INFO) << "Update: " << id.name_full() << " "
                << update.is_updated_transform()
                << update.is_updated_geometry()
                << update.is_updated_shading();

      if (id.is_a(&RNA_Object)) {
        Object *object = (Object *)id.ptr.data;
        if (!supported_object(object)) {
          continue;
        }
        add_update_object(object,
                          update.is_updated_geometry(),
                          update.is_updated_transform(),
                          update.is_updated_shading());
        continue;
      }

      if (id.is_a(&RNA_Material)) {
        if (update.is_updated_shading()) {
          Material *material = (Material *)id.ptr.data;
          update_material(material);
        }
        continue;
      }
      
      if (id.is_a(&RNA_Collection)) {
        if (update.is_updated_transform() && update.is_updated_geometry()) {
          update_collection();
        }
        continue;
      }
    }
    return;
  }

  update_collection();
  
  is_populated = true;
}

HdMeshTopology BlenderSceneDelegate::GetMeshTopology(SdfPath const& id)
{
  LOG(INFO) << "GetMeshTopology: " << id.GetAsString();
  ObjectData &obj_data = objects[id];
  return HdMeshTopology(PxOsdOpenSubdivTokens->catmullClark, HdTokens->rightHanded,
                        obj_data.get_data<VtIntArray>(TfToken("faceCounts")),
                        obj_data.get_data<VtIntArray>(HdTokens->pointsIndices));
}

VtValue BlenderSceneDelegate::Get(SdfPath const& id, TfToken const& key)
{
  LOG(INFO) << "Get: " << id.GetAsString() << " [" << key.GetString() << "]";
  
  VtValue ret;
  ObjectData *obj_data = object_data(id);
  if (obj_data) {
    if (obj_data->has_data(key)) {
      ret = obj_data->get_data(key);
    }
  }
  else if (key.GetString() == "MaterialXFilename") {
    MaterialData &mat_data = materials[id];
    if (!mat_data.mtlx_path.GetResolvedPath().empty()) {
      ret = mat_data.mtlx_path;
    }
  }
  else if (key == HdStRenderBufferTokens->stormMsaaSampleCount) {
    // TODO: temporary value, it should be delivered through Python UI
    ret = 16;
  }
  return ret;
}

HdPrimvarDescriptorVector BlenderSceneDelegate::GetPrimvarDescriptors(SdfPath const& id, HdInterpolation interpolation)
{
  LOG(INFO) << "GetPrimvarDescriptors: " << id.GetAsString() << " " << interpolation;
  HdPrimvarDescriptorVector primvars;
  ObjectData &obj_data = objects[id];
  if (interpolation == HdInterpolationVertex) {
    if (obj_data.has_data(HdTokens->points)) {
      primvars.emplace_back(HdTokens->points, interpolation, HdPrimvarRoleTokens->point);
    }
  }
  else if (interpolation == HdInterpolationFaceVarying) {
    if (obj_data.has_data(HdTokens->normals)) {
      primvars.emplace_back(HdTokens->normals, interpolation, HdPrimvarRoleTokens->normal);
    }
    if (obj_data.has_data(HdPrimvarRoleTokens->textureCoordinate)) {
      primvars.emplace_back(HdPrimvarRoleTokens->textureCoordinate, interpolation, HdPrimvarRoleTokens->textureCoordinate);
    }
  }
  return primvars;
}

SdfPath BlenderSceneDelegate::GetMaterialId(SdfPath const & rprimId)
{
  SdfPath ret;
  ObjectData *obj_data = object_data(rprimId);
  if (obj_data && obj_data->has_data(TfToken("materialId"))) {
    ret = obj_data->get_data<SdfPath>(TfToken("materialId"));
  }

  LOG(INFO) << "GetMaterialId [" << rprimId.GetAsString() << "] = " << ret.GetAsString();
  return ret;
}

VtValue BlenderSceneDelegate::GetMaterialResource(SdfPath const& materialId)
{
  LOG(INFO) << "GetMaterialResource: " << materialId.GetAsString();
  return VtValue();
}

GfMatrix4d BlenderSceneDelegate::GetTransform(SdfPath const& id)
{
  LOG(INFO) << "GetTransform: " << id.GetAsString();

  return objects[id].transform();
}

VtValue BlenderSceneDelegate::GetLightParamValue(SdfPath const& id, TfToken const& key)
{
  LOG(INFO) << "GetLightParamValue: " << id.GetAsString() << " [" << key.GetString() << "]";
  VtValue ret;
  ObjectData *obj_data = object_data(id);
  if (obj_data) {
    if (obj_data->has_data(key)) {
      ret = obj_data->get_data(key);
    }
    else if (key == HdLightTokens->exposure) {
      // TODO: temporary value, it should be delivered through Python UI
      ret = 1.0f;
    }
  }
  return ret;
}

} // namespace blender::render::hydra
