/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <map>

#include <pxr/base/vt/value.h>
#include <pxr/imaging/hd/tokens.h>

#include "DNA_light_types.h"

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

namespace usdhydra {

class LightExport
{
public:
  LightExport(BL::Light &b_light);
  pxr::VtValue intensity();
  pxr::VtValue width();
  pxr::VtValue height();
  pxr::VtValue radius();
  pxr::VtValue color();
  pxr::VtValue angle();
  pxr::VtValue shapingConeAngle();
  pxr::VtValue shapingConeSoftness();
  pxr::VtValue treatAsPoint();
  pxr::TfToken type();

private:
  Light *light;
};

} // namespace usdhydra
