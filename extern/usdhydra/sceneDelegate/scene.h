/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <pxr/base/gf/camera.h>

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

namespace usdhydra {

class SceneExport
{
public:
  SceneExport(BL::Depsgraph &b_depsgraph)
    : b_depsgraph(b_depsgraph)
    , b_scene(b_depsgraph.scene())
  {}
  pxr::GfCamera gfCamera();
  std::pair<int, int> resolution();

private:
  BL::Depsgraph &b_depsgraph;
  BL::Scene b_scene;
};

} // namespace usdhydra
