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

namespace usdhydra {

MeshExport::MeshExport(BL::Object &b_object)
{
  BL::Mesh &b_mesh = (BL::Mesh &)b_object.data();
  //Object *obj = (Object *)b_object.ptr.data;
  //mesh = BKE_object_get_evaluated_mesh(obj);
  mesh = ((Mesh *)b_mesh.ptr.data);

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
  blender::Span<MLoopTri> tris = mesh->looptris();
  for (MLoopTri tri : tris) {
    ret.push_back(tri.tri[0]);
    ret.push_back(tri.tri[1]);
    ret.push_back(tri.tri[2]);
  }
  return ret;
}

VtVec3fArray MeshExport::vertices()
{
  VtVec3fArray ret;
  ret.reserve(mesh->totvert);
  blender::Span<MVert> verts = mesh->verts();
  for (MVert v : verts) {
    ret.push_back(GfVec3f(v.co));
  }
  return ret;
}

VtVec3fArray MeshExport::normals()
{
  const float(*lnors)[3] = static_cast<float(*)[3]>(CustomData_get_layer(&mesh->ldata, CD_NORMAL));
  const blender::Span<MPoly> polys = mesh->polys();
  const blender::Span<MLoop> loops = mesh->loops();

  VtVec3fArray ret;
  ret.reserve(mesh->totloop);

  if (lnors != nullptr) {
    /* Export custom loop normals. */
    for (int loop_idx = 0, totloop = mesh->totloop; loop_idx < totloop; ++loop_idx) {
      ret.push_back(pxr::GfVec3f(lnors[loop_idx]));
    }
  }
  else {
    /* Compute the loop normals based on the 'smooth' flag. */
    const float(*vert_normals)[3] = BKE_mesh_vertex_normals_ensure(mesh);
    const float(*face_normals)[3] = BKE_mesh_poly_normals_ensure(mesh);
    for (const int i : polys.index_range()) {
      const MPoly &poly = polys[i];

      if ((poly.flag & ME_SMOOTH) == 0) {
        /* Flat shaded, use common normal for all verts. */
        pxr::GfVec3f pxr_normal(face_normals[i]);
        for (int loop_idx = 0; loop_idx < poly.totloop; ++loop_idx) {
          ret.push_back(pxr_normal);
        }
      }
      else {
        /* Smooth shaded, use individual vert normals. */
        for (const MLoop &loop : loops.slice(poly.loopstart, poly.totloop)) {
          ret.push_back(pxr::GfVec3f(vert_normals[loop.v]));
        }
      }
    }
  }

  return ret;
}

} // namespace usdhydra
