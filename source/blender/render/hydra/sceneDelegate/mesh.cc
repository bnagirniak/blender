/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <iostream>

#include "BKE_mesh.h"
#include "BKE_mesh_runtime.h"
#include "BKE_object.h"
#include "BKE_customdata.h"

#include <BLI_span.hh>

#include "mesh.h"

using namespace pxr;

namespace blender::render::hydra {

MeshExport::MeshExport(BL::Mesh &b_mesh)
  : mesh((Mesh *)b_mesh.ptr.data)
{
}

VtIntArray MeshExport::faceVertexCounts()
{
  int trisLen = BKE_mesh_runtime_looptri_len(mesh);
  VtIntArray ret(trisLen);
  for (int i = 0; i < trisLen; ++i) {
    ret[i] = 3; 
  }
  return ret;
}

VtIntArray MeshExport::faceVertexIndices()
{
  VtIntArray ret;
  blender::Span<MLoopTri> loopTris = mesh->looptris();
  blender::Span<MLoop> loops = mesh->loops();
  for (MLoopTri lt : loopTris) {
    ret.push_back(loops[lt.tri[0]].v);
    ret.push_back(loops[lt.tri[1]].v);
    ret.push_back(loops[lt.tri[2]].v);
  }
  return ret;
}

VtVec3fArray MeshExport::vertices()
{
  VtVec3fArray ret;
  ret.reserve(mesh->totvert);
  blender::Span<blender::float3> verts = mesh->vert_positions();
  for (blender::float3 v : verts) {
    ret.push_back(GfVec3f(v.x, v.y, v.z));
  }
  return ret;
}

VtVec3fArray MeshExport::normals()
{
  BKE_mesh_calc_normals_split(mesh);
  const float(*lnors)[3] = (float(*)[3])CustomData_get_layer(&mesh->ldata, CD_NORMAL);
  blender::Span<MLoopTri> loopTris = mesh->looptris();

  VtVec3fArray ret;
  if (lnors) {
    for (MLoopTri lt : loopTris) {
      ret.push_back(pxr::GfVec3f(lnors[lt.tri[0]]));
      ret.push_back(pxr::GfVec3f(lnors[lt.tri[1]]));
      ret.push_back(pxr::GfVec3f(lnors[lt.tri[2]]));
    }
  }
  return ret;
}

VtVec2fArray MeshExport::uvs()
{
  const float(*luvs)[2] = (float(*)[2])CustomData_get_layer(&mesh->ldata, CD_PROP_FLOAT2);
  blender::Span<MLoopTri> loopTris = mesh->looptris();

  VtVec2fArray ret;
  if (luvs) {
    for (MLoopTri lt : loopTris) {
      ret.push_back(pxr::GfVec2f(luvs[lt.tri[0]]));
      ret.push_back(pxr::GfVec2f(luvs[lt.tri[1]]));
      ret.push_back(pxr::GfVec2f(luvs[lt.tri[2]]));
    }
  }
  return ret;
}

MeshData::MeshData(Mesh *mesh)
{
  /* faceVertexCounts */
  int tris_len = BKE_mesh_runtime_looptri_len(mesh);
  faceVertexCounts.reserve(tris_len);
  for (int i = 0; i < tris_len; ++i) {
    faceVertexCounts.push_back(3);
  }

  /* faceVertexIndices */
  blender::Span<MLoopTri> loopTris = mesh->looptris();
  blender::Span<MLoop> loops = mesh->loops();
  faceVertexIndices.reserve(loopTris.size() * 3);
  for (MLoopTri lt : loopTris) {
    faceVertexIndices.push_back(loops[lt.tri[0]].v);
    faceVertexIndices.push_back(loops[lt.tri[1]].v);
    faceVertexIndices.push_back(loops[lt.tri[2]].v);
  }

  /* vertices */
  vertices.reserve(mesh->totvert);
  blender::Span<blender::float3> verts = mesh->vert_positions();
  for (blender::float3 v : verts) {
    vertices.push_back(GfVec3f(v.x, v.y, v.z));
  }

  /* normals */
  BKE_mesh_calc_normals_split(mesh);
  const float(*lnors)[3] = (float(*)[3])CustomData_get_layer(&mesh->ldata, CD_NORMAL);
  normals.reserve(loopTris.size() * 3);
  if (lnors) {
    for (MLoopTri lt : loopTris) {
      normals.push_back(pxr::GfVec3f(lnors[lt.tri[0]]));
      normals.push_back(pxr::GfVec3f(lnors[lt.tri[1]]));
      normals.push_back(pxr::GfVec3f(lnors[lt.tri[2]]));
    }
  }

  /* UVs*/
  const float(*luvs)[2] = (float(*)[2])CustomData_get_layer(&mesh->ldata, CD_PROP_FLOAT2);
  uvs.reserve(loopTris.size() * 3);
  if (luvs) {
    for (MLoopTri lt : loopTris) {
      uvs.push_back(pxr::GfVec2f(luvs[lt.tri[0]]));
      uvs.push_back(pxr::GfVec2f(luvs[lt.tri[1]]));
      uvs.push_back(pxr::GfVec2f(luvs[lt.tri[2]]));
    }
  }
}

}  // namespace blender::render::hydra
