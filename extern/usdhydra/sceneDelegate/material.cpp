/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */
#include <math.h>

#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec2f.h>

#include "material.h"

using namespace pxr;

namespace usdhydra {

MaterialExport::MaterialExport(BL::Material &b_material)
  : material((Material *)b_material.ptr.data)
{
}

} // namespace usdhydra
