/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <pxr/usd/sdf/assetPath.h>

#include "DNA_material_types.h"

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

namespace usdhydra {

class MaterialExport
{
public:
  MaterialExport(BL::Object &b_object);
  MaterialExport(BL::Material &b_material);

  operator bool();
  std::string name();
  pxr::SdfAssetPath exportMX();

private:
  Material *material;
};

} // namespace usdhydra
