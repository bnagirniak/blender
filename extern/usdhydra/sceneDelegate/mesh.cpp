/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <iostream>

#include <BLI_span.hh>

#include "mesh.h"

using namespace pxr;

namespace usdhydra {

MeshExport::MeshExport(BL::Mesh &b_mesh)
  : b_mesh(b_mesh)
  , mesh((Mesh *)b_mesh.ptr.data)
{
}

VtIntArray MeshExport::faceVertexCounts()
{
  VtIntArray ret = {3, 3};
  return ret;
}

VtIntArray MeshExport::faceVertexIndices()
{
  VtIntArray ret = {0, 1, 3, 0, 3, 2};
  return ret;
}

VtVec3fArray MeshExport::vertices()
{
  VtVec3fArray ret; // = {{-1, -1, 0}, {1, -1, 0}, {-1, 1, 0}, {1, 1, 0}};
  blender::Span<MVert> verts = mesh->verts();
  for (MVert v : verts) {
    ret.push_back(GfVec3f(v.co));
  }

  return ret;
}

VtVec3fArray MeshExport::normals()
{
  VtVec3fArray ret = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}};
  return ret;
}

} // namespace usdhydra
