/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <Python.h>

#include <pxr/usd/usd/stage.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

namespace hdusd {

class BlenderSession {
public:
  BlenderSession(BL::RenderEngine &b_engine);
  ~BlenderSession();

  void reset(int stageId);
  void render(BL::Depsgraph &b_depsgraph);
  void view_draw(BL::Depsgraph &b_depsgraph, BL::Context &b_context);

public:
  BL::RenderEngine b_engine;
  //BL::BlendData b_data;

  std::unique_ptr<pxr::UsdImagingGLEngine> imagingGLEngine;
  pxr::UsdImagingGLRenderParams render_params;
  pxr::UsdStageRefPtr stage;
};

PyObject *addPythonSubmodule_session(PyObject *mod);

}   // namespace hdusd
