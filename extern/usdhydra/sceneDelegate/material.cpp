/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */
#include <math.h>

#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec2f.h>

#include "material.h"

using namespace pxr;

namespace usdhydra {

MaterialExport::MaterialExport(BL::Object &b_object)
  : material(nullptr)
{
  if (b_object.material_slots.empty()) {
    return;
  }
    
  BL::Material b_material = b_object.material_slots[0].material();
  if (!b_material) {
    return;
  }
  material = (Material *)b_material.ptr.data;
}

MaterialExport::operator bool() {
  return bool(material);
}

std::string MaterialExport::name() {
  return material->id.name + 2;
}

} // namespace usdhydra
