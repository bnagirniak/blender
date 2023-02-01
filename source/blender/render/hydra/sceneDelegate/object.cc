/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include "BKE_object.h"

#include "object.h"

using namespace pxr;

namespace blender::render::hydra {

MeshExport ObjectExport::meshExport()
{
  return MeshExport((BL::Mesh &)b_object.data());
}

LightExport ObjectExport::lightExport()
{
  return LightExport((BL::Light &)b_object.data());
}

MaterialExport ObjectExport::materialExport()
{
  return MaterialExport(b_object);
}

pxr::GfMatrix4d ObjectExport::transform()
{
  auto m = b_object.matrix_world();
  return pxr::GfMatrix4d(
    m[0], m[1], m[2], m[3],
    m[4], m[5], m[6], m[7],
    m[8], m[9], m[10], m[11],
    m[12], m[13], m[14], m[15]);
}

std::string ObjectExport::name()
{
  return b_object.name_full();
}

BL::Object::type_enum ObjectExport::type()
{
  return b_object.type();
}

ObjectData::ObjectData(Object *object)
  : object(object)
{

}

std::string ObjectData::name()
{
  return object->id.name + 2;
}

TfToken ObjectData::prim_type()
{
  TfToken ret;
  Light *light;
  switch (object->type) {
    case OB_MESH:
    case OB_MBALL:
    case OB_SURF:
    case OB_FONT:
      ret = HdPrimTypeTokens->mesh;
      break;

    case OB_LAMP:
      light = (Light *)object->data;
      switch (light->type) {
        case LA_AREA:
          switch (light->area_shape) {
            case LA_AREA_SQUARE:
            case LA_AREA_RECT:
              ret = pxr::HdPrimTypeTokens->rectLight;
              break;

            case LA_AREA_DISK:
            case LA_AREA_ELLIPSE:
              ret = pxr::HdPrimTypeTokens->diskLight;
              break;

            default:
              ret = pxr::HdPrimTypeTokens->rectLight;
          }
          break;

        case LA_SUN:
          ret = pxr::HdPrimTypeTokens->distantLight;
          break;

        case LA_LOCAL:
        case LA_SPOT:
          ret = pxr::HdPrimTypeTokens->sphereLight;
          break;

        default:
          ret = pxr::HdPrimTypeTokens->sphereLight;
      }
      break;

    default:
      break;
  }
  return ret;
}

GfMatrix4d ObjectData::transform()
{
  float *m = (float *)object->object_to_world;
  return pxr::GfMatrix4d(
    m[0], m[1], m[2], m[3],
    m[4], m[5], m[6], m[7],
    m[8], m[9], m[10], m[11],
    m[12], m[13], m[14], m[15]);
}

std::string ObjectData::path_name()
{
  char str[32];
  snprintf(str, 32, "%016llX", (uint64_t)object);
  return str;
}

VtValue &ObjectData::get_data(const TfToken &key)
{
  return data[key];
}

bool ObjectData::has_data(const TfToken &key)
{
  return data.find(key) != data.end();
}

void ObjectData::set_as_mesh()
{
}

void ObjectData::set_as_meshable()
{
}

void ObjectData::set_as_light()
{
}

}  // namespace blender::render::hydra
