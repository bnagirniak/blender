/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <map>

#include <pxr/usd/sdf/assetPath.h>
#include <pxr/usd/sdf/path.h>

#include "DNA_material_types.h"

namespace blender::render::hydra {

class MaterialData {
 public:
  MaterialData();
  MaterialData(Material *material);

  std::string name();
  void export_mtlx();
  pxr::SdfAssetPath mtlx_path;

 private:
  Material *material;
};

using MaterialDataMap = std::map<pxr::SdfPath, MaterialData>;

} // namespace blender::render::hydra
