/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include "finalEngine.h"
#include "finalEngineGL.h"

namespace blender::render::hydra {

class FinalEngineGL : public FinalEngine {
public:
  using FinalEngine::FinalEngine;
  void render(BL::Depsgraph& b_depsgraph) override;
};

}   // namespace blender::render::hydra
