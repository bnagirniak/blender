/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <map>

#include <pxr/base/gf/matrix4d.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/base/vt/value.h>

//#include "MEM_guardedalloc.h"
//#include "RNA_blender_cpp.h"

#include "material.h"

namespace blender::render::hydra {

class ObjectData {
public:
  ObjectData();
  ObjectData(Object *object);

  std::string name();
  int type();
  pxr::TfToken prim_type();
  pxr::GfMatrix4d transform();
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
  void set_as_camera();
};

using ObjectDataMap = std::map<pxr::SdfPath, ObjectData>;

template<class T>
const T &ObjectData::get_data(const pxr::TfToken &key)
{
  return get_data(key).Get<T>();
}

} // namespace blender::render::hydra
