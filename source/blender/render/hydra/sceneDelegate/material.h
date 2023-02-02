/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <map>

#include <pxr/usd/sdf/assetPath.h>
#include <pxr/usd/sdf/path.h>

#include "DNA_material_types.h"

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

namespace blender::render::hydra {

class MaterialExport
{
public:
  MaterialExport(BL::Object &b_object);
  MaterialExport(BL::Material &b_material);

  operator bool();
  std::string name();
  pxr::SdfAssetPath export_mtlx();

private:
  Material *material;
};

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
