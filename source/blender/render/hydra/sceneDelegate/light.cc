/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */
#include <math.h>

#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec2f.h>

#include "light.h"

using namespace pxr;

namespace blender::render::hydra {

LightExport::LightExport(BL::Light &b_light)
  : light((Light *)b_light.ptr.data)
{
}

VtValue LightExport::intensity() {
  return VtValue(light->energy);
}

VtValue LightExport::width() {
  if (light->area_shape == LA_AREA_SQUARE)
    return VtValue(light->area_size);
  
  if (light->area_shape == LA_AREA_RECT)
    return VtValue(light->area_size);
  
  return VtValue();
}

VtValue LightExport::height() {
  if (light->area_shape == LA_AREA_SQUARE)
    return VtValue(light->area_size);

  if (light->area_shape == LA_AREA_RECT)
    return VtValue(light->area_sizey);

  return VtValue();
}

VtValue LightExport::radius() {
  if (light->type == LA_LOCAL || light->type == LA_SPOT)
    return VtValue(light->area_size / 2); // light.area_size is diameter

  if (light->area_shape == LA_AREA_DISK)
    return VtValue(light->area_size / 2); // light.area_size is diameter

  if (light->area_shape == LA_AREA_ELLIPSE)
    return VtValue((light->area_size  + light->area_sizey) / 4); // average of light.size is diameter

  return VtValue();
}

VtValue LightExport::color() {
  return VtValue(GfVec3f(light->r, light->g, light->b));
}

VtValue LightExport::angle() {
  return VtValue(light->sun_angle * 180.0 / M_PI);
}

VtValue LightExport::shapingConeAngle() {
  if (light->type == LA_SPOT)
    return VtValue(light->spotsize / 2);

  return VtValue();
}

VtValue LightExport::shapingConeSoftness() {
  if (light->type == LA_SPOT)
    return VtValue(light->spotblend);

  return VtValue();
}

VtValue LightExport::treatAsPoint() {
  if (light->type == LA_SPOT)
    return VtValue(1);

  return VtValue();
}

TfToken LightExport::type() {
  if (light->type == LA_AREA) {
    switch (light->area_shape) {
      case LA_AREA_SQUARE: return pxr::HdPrimTypeTokens->rectLight;
      case LA_AREA_RECT: return pxr::HdPrimTypeTokens->rectLight;
      case LA_AREA_DISK: return pxr::HdPrimTypeTokens->diskLight;
      case LA_AREA_ELLIPSE: return pxr::HdPrimTypeTokens->diskLight;
      default: return pxr::HdPrimTypeTokens->rectLight;
    }
  }
  else {
    switch (light->type) {
      case LA_SUN: return pxr::HdPrimTypeTokens->distantLight;
      case LA_LOCAL: return pxr::HdPrimTypeTokens->sphereLight;
      case LA_SPOT: return pxr::HdPrimTypeTokens->sphereLight;
      default: return pxr::HdPrimTypeTokens->sphereLight;
    }
  }
}

} // namespace blender::render::hydra
