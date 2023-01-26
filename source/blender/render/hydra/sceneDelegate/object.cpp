/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include "object.h"

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

} // namespace blender::render::hydra
