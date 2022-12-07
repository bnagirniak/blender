/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>

#include "DNA_mesh_types.h"

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

namespace usdhydra {

class MeshExport
{
public:
  MeshExport(BL::Object &b_object);
  pxr::VtIntArray faceVertexCounts();
  pxr::VtIntArray faceVertexIndices();
  pxr::VtVec3fArray vertices();
  pxr::VtVec3fArray normals();

private:
  Mesh *mesh;
};

} // namespace usdhydra
