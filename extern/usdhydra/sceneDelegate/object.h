/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <pxr/base/gf/matrix4d.h>

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

#include "mesh.h"
#include "light.h"

namespace usdhydra {

class ObjectExport
{
public:
  ObjectExport(BL::Object &b_object)
    : b_object(b_object)
  {}
  inline MeshExport meshExport();
  inline LightExport lightExport();

  pxr::GfMatrix4d transform();

private:
  BL::Object &b_object;
};

inline MeshExport ObjectExport::meshExport()
{
  return MeshExport((BL::Mesh &)b_object.data());
}

inline LightExport ObjectExport::lightExport()
{
  return LightExport((BL::Light &)b_object.data());
}

} // namespace usdhydra
