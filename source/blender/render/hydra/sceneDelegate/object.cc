/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/imaging/hd/light.h>
#include <pxr/imaging/hd/tokens.h>
#include <pxr/usd/usdLux/tokens.h>

#include "DNA_light_types.h"
#include "BKE_object.h"
#include "BKE_lib_id.h"
#include "BKE_material.h"
#include "BKE_light.h"
#include "BKE_mesh.h"
#include "BKE_mesh_runtime.h"

#include "object.h"

PXR_NAMESPACE_OPEN_SCOPE
TF_DEFINE_PUBLIC_TOKENS(HdBlenderTokens, HD_BLENDER_TOKENS);
PXR_NAMESPACE_CLOSE_SCOPE

using namespace pxr;

namespace blender::render::hydra {

ObjectData::ObjectData()
  : object(nullptr)
{
  
}

ObjectData::ObjectData(Object *object)
  : object(object)
{
  switch (object->type) {
    case OB_MESH:
      set_as_mesh();
      break;

    case OB_SURF:
    case OB_FONT:
    case OB_MBALL:
      set_as_meshable();
      break;

    case OB_LAMP:
      set_as_light();
      break;

    case OB_CAMERA:
      set_as_camera();
      break;

    default:
      break;
  }
}

std::string ObjectData::name()
{
  char str[MAX_ID_FULL_NAME];
  BKE_id_full_name_get(str, (ID *)object, 0);
  return str;
}

int ObjectData::type()
{
  return object->type;
}

TfToken ObjectData::prim_type()
{
  TfToken ret;
  Light *light;
  switch (object->type) {
    case OB_MESH:
    case OB_SURF:
    case OB_FONT:
    case OB_MBALL:
      ret = HdPrimTypeTokens->mesh;
      break;

    case OB_LAMP:
      light = (Light *)object->data;
      switch (light->type) {
        case LA_LOCAL:
        case LA_SPOT:
          ret = HdPrimTypeTokens->sphereLight;
          break;

        case LA_SUN:
          ret = HdPrimTypeTokens->distantLight;
          break;

        case LA_AREA:
          switch (light->area_shape) {
            case LA_AREA_SQUARE:
            case LA_AREA_RECT:
              ret = HdPrimTypeTokens->rectLight;
              break;

            case LA_AREA_DISK:
            case LA_AREA_ELLIPSE:
              ret = HdPrimTypeTokens->diskLight;
              break;

            default:
              ret = HdPrimTypeTokens->rectLight;
          }
          break;

        default:
          ret = HdPrimTypeTokens->sphereLight;
      }
      break;

    case OB_CAMERA:
      ret = HdPrimTypeTokens->camera;
      break;

    default:
      break;
  }
  return ret;
}

GfMatrix4d ObjectData::transform()
{
  float *m = (float *)object->object_to_world;
  return GfMatrix4d(
    m[0], m[1], m[2], m[3],
    m[4], m[5], m[6], m[7],
    m[8], m[9], m[10], m[11],
    m[12], m[13], m[14], m[15]);
}

Material *ObjectData::material()
{
  if (BKE_object_material_count_eval(object) == 0) {
    return nullptr;
  }
  return BKE_object_material_get_eval(object, object->actcol);
}

VtValue &ObjectData::get_data(TfToken const &key)
{
  return data[key];
}

bool ObjectData::has_data(TfToken const &key)
{
  return data.find(key) != data.end();
}

void ObjectData::set_material_id(SdfPath const &id)
{
  if (id.IsEmpty()) {
    data.erase(HdBlenderTokens->materialId);
  }
  else {
    data[HdBlenderTokens->materialId] = id;
  }
}

void ObjectData::set_as_mesh()
{
  Mesh *mesh = (Mesh *)object->data;
  BKE_mesh_calc_normals_split(mesh);
  int tris_len = BKE_mesh_runtime_looptri_len(mesh);
  blender::Span<MLoopTri> loopTris = mesh->looptris();

  /* faceVertexCounts */
  data[HdBlenderTokens->faceCounts] = VtIntArray(tris_len, 3);

  /* faceVertexIndices */
  VtIntArray faceVertexIndices;
  blender::Span<MLoop> loops = mesh->loops();
  faceVertexIndices.reserve(loopTris.size() * 3);
  for (MLoopTri lt : loopTris) {
    faceVertexIndices.push_back(loops[lt.tri[0]].v);
    faceVertexIndices.push_back(loops[lt.tri[1]].v);
    faceVertexIndices.push_back(loops[lt.tri[2]].v);
  }
  data[HdTokens->pointsIndices] = faceVertexIndices;

  /* vertices */
  VtVec3fArray vertices;
  vertices.reserve(mesh->totvert);
  blender::Span<blender::float3> verts = mesh->vert_positions();
  for (blender::float3 v : verts) {
    vertices.push_back(GfVec3f(v.x, v.y, v.z));
  }
  data[HdTokens->points] = vertices;

  /* normals */
  const float(*lnors)[3] = (float(*)[3])CustomData_get_layer(&mesh->ldata, CD_NORMAL);
  if (lnors) {
    VtVec3fArray normals;
    normals.reserve(loopTris.size() * 3);
    for (MLoopTri lt : loopTris) {
      normals.push_back(GfVec3f(lnors[lt.tri[0]]));
      normals.push_back(GfVec3f(lnors[lt.tri[1]]));
      normals.push_back(GfVec3f(lnors[lt.tri[2]]));
    }
    data[HdTokens->normals] = normals;
  }

  /* UVs*/
  const float(*luvs)[2] = (float(*)[2])CustomData_get_layer(&mesh->ldata, CD_PROP_FLOAT2);
  if (luvs) {
    VtVec2fArray uvs;
    uvs.reserve(loopTris.size() * 3);
    for (MLoopTri lt : loopTris) {
      uvs.push_back(GfVec2f(luvs[lt.tri[0]]));
      uvs.push_back(GfVec2f(luvs[lt.tri[1]]));
      uvs.push_back(GfVec2f(luvs[lt.tri[2]]));
    }
    data[HdPrimvarRoleTokens->textureCoordinate] = uvs;
  }
}

void ObjectData::set_as_meshable()
{
}

void ObjectData::set_as_light()
{
  Light *light = (Light *)object->data;
  data[HdLightTokens->intensity] = light->energy;
  data[HdLightTokens->color] = GfVec3f(light->r, light->g, light->b);

  switch (light->type) {
    case LA_LOCAL:
      data[HdLightTokens->radius] = light->area_size / 2;
      break;

    case LA_SUN:
      data[HdLightTokens->angle] = light->sun_angle * 180.0 / M_PI;
      break;

    case LA_SPOT:
      data[HdLightTokens->shapingConeAngle] = light->spotsize / 2;
      data[HdLightTokens->shapingConeSoftness] = light->spotblend;
      data[UsdLuxTokens->treatAsPoint] = 1;
      break;

    case LA_AREA:
      switch (light->area_shape) {
        case LA_AREA_SQUARE:
          data[HdLightTokens->width] = light->area_size;
          data[HdLightTokens->height] = light->area_size;
          break;
        case LA_AREA_RECT:
          data[HdLightTokens->width] = light->area_size;
          data[HdLightTokens->height] = light->area_sizey;
          break;

        case LA_AREA_DISK:
          data[HdLightTokens->radius] = light->area_size / 2;
          break;

        case LA_AREA_ELLIPSE:
          data[HdLightTokens->radius] = (light->area_size + light->area_sizey) / 4;
          break;

        default:
          break;
      }
      break;

    default:
      break;
  }
}

void ObjectData::set_as_camera()
{
}

}  // namespace blender::render::hydra
