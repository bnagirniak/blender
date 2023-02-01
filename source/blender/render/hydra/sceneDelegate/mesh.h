/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec2f.h>

#include "DNA_mesh_types.h"

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

namespace blender::render::hydra {

class MeshExport
{
public:
  MeshExport(BL::Mesh &b_mesh);
  pxr::VtIntArray faceVertexCounts();
  pxr::VtIntArray faceVertexIndices();
  pxr::VtVec3fArray vertices();
  pxr::VtVec3fArray normals();
  pxr::VtVec2fArray uvs();

private:
  Mesh *mesh;
};


struct MeshData {
  MeshData(Mesh *mesh);

  pxr::VtIntArray faceVertexCounts;
  pxr::VtIntArray faceVertexIndices;
  pxr::VtVec3fArray vertices;
  pxr::VtVec3fArray normals;
  pxr::VtVec2fArray uvs;
};
} // namespace blender::render::hydra
