/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <map>

#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/tf/hashmap.h>
#include <pxr/usd/sdf/path.h>

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

#include "mesh.h"
#include "light.h"
#include "material.h"

namespace blender::render::hydra {

class ObjectExport
{
public:
  ObjectExport(BL::Object b_object, BL::Depsgraph &b_depsgraph)
    : b_object(b_object)
    , b_depsgraph(b_depsgraph)
  {}
  MeshExport meshExport();
  LightExport lightExport();
  MaterialExport materialExport();

  pxr::GfMatrix4d transform();
  std::string name();
  BL::Object::type_enum type();

private:
  BL::Object b_object;
  BL::Depsgraph &b_depsgraph;
};

class ObjectData {
public:
  ObjectData();
  ObjectData(Object *object);

  std::string name();
  int type();
  pxr::TfToken prim_type();
  pxr::GfMatrix4d transform();
  std::string path_name();
  Material *material();

  pxr::VtValue &get_data(const pxr::TfToken &key);
  template<class T>
  const T &get_data(const pxr::TfToken &key);
  bool has_data(const pxr::TfToken &key);

  void set_material_id(pxr::SdfPath const &id);

 private:
  Object *object;
  std::map<pxr::TfToken, pxr::VtValue> data;

  void set_as_mesh();
  void set_as_meshable();
  void set_as_light();
};

using ObjectDataMap = std::map<pxr::SdfPath, ObjectData>;

template<class T>
const T &ObjectData::get_data(const pxr::TfToken &key)
{
  return get_data(key).Get<T>();
}

} // namespace blender::render::hydra
