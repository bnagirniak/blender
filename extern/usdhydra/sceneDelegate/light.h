/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

namespace usdhydra {

class LightExport
{
public:
  LightExport(BL::Light &b_light)
    : b_light(b_light)
  {}

private:
  BL::Light &b_light;
};


} // namespace usdhydra
