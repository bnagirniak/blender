/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <pxr/base/gf/matrix4d.h>

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

#include "mesh.h"
#include "light.h"
#include "material.h"

namespace usdhydra {

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

} // namespace usdhydra
