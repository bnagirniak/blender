/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <memory>

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/stageCache.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

namespace hdusd {

extern std::unique_ptr<pxr::UsdStageCache> stageCache;
extern std::unique_ptr<pxr::UsdImagingGLEngine> imagingGLEngine;

class BlenderSession {
public:
  BlenderSession(BL::RenderEngine &b_engine, BL::BlendData &b_data);
  ~BlenderSession();

  BL::RenderEngine b_engine;
  BL::BlendData b_data;
};

}   // namespace hdusd
