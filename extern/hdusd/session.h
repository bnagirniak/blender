/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include "RNA_blender_cpp.h"

class BlenderSession {
public:
  BlenderSession(BL::RenderEngine &b_engine, BL::BlendData &b_data);
  ~BlenderSession();

  BL::RenderEngine b_engine;
  BL::BlendData b_data;
};
