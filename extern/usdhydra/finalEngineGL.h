/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include "finalEngine.h"
#include "finalEngineGL.h"

namespace usdhydra {

class FinalEngineGL : public FinalEngine {
public:
  using FinalEngine::FinalEngine;
  void sync(BL::Depsgraph &b_depsgraph, BL::Context &b_context, pxr::HdRenderSettingsMap &renderSettings) override;
  void render(BL::Depsgraph& b_depsgraph) override;
};

}   // namespace usdhydra
