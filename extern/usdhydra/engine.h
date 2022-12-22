/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <chrono>

#include <Python.h>

#include <pxr/imaging/hd/engine.h>
#include <pxr/imaging/hdx/freeCameraSceneDelegate.h>

#include <pxr/usd/usd/stage.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

#include "sceneDelegate/blenderSceneDelegate.h"
#include "renderTaskDelegate.h"

namespace usdhydra {

class Engine {
public:
  Engine(BL::RenderEngine &b_engine, const std::string &delegateId);
  virtual ~Engine();

  virtual void sync(BL::Depsgraph &b_depsgraph, BL::Context &b_context, pxr::HdRenderSettingsMap &renderSettings) = 0;
  pxr::UsdStageRefPtr getStage();

protected:
  void exportScene(BL::Depsgraph &b_depsgraph, BL::Context &b_context);

  template <typename T>
  float getRendererPercentDone(T &renderer);

protected:
  BL::RenderEngine b_engine;

  HdPluginRenderDelegateUniqueHandle renderDelegate;
  std::unique_ptr<HdRenderIndex> renderIndex;
  std::unique_ptr<BlenderSceneDelegate> sceneDelegate;
  std::unique_ptr<RenderTaskDelegate> renderTaskDelegate;
  std::unique_ptr<HdxFreeCameraSceneDelegate> freeCameraDelegate;
  HdEngine _engine;

  std::string delegateId;
  pxr::HdRenderSettingsMap renderSettings;
  pxr::UsdStageRefPtr stage;
};

template <typename T>
float Engine::getRendererPercentDone(T &renderer)
{
  float percent = 0.0;

  VtDictionary render_stats = renderer.GetRenderStats();
  auto it = render_stats.find("percentDone");
  if (it != render_stats.end()) {
    percent = (float)it->second.UncheckedGet<double>();
  }

  return round(percent * 10.0f) / 10.0f;
}

inline pxr::UsdStageRefPtr Engine::getStage()
{
  return stage;
}

PyObject *addPythonSubmodule_engine(PyObject *mod);

}   // namespace usdhydra
