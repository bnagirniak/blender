/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <map>

#include <pxr/base/vt/value.h>
#include <pxr/imaging/hd/tokens.h>

#include "DNA_material_types.h"

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

namespace usdhydra {

class MaterialExport
{
public:
  MaterialExport(BL::Material &b_material);

private:
  Material *material;
};

} // namespace usdhydra
