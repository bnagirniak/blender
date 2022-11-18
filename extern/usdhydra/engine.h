/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <chrono>

#include <Python.h>

#include <pxr/usd/usd/stage.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

namespace usdhydra {

class Engine {
public:
  Engine(BL::RenderEngine &b_engine, const char* delegateId);
  virtual ~Engine();

  virtual void sync(BL::Depsgraph &b_depsgraph, BL::Context &b_context, pxr::HdRenderSettingsMap &renderSettings) = 0;

protected:
  void exportScene(BL::Depsgraph &b_depsgraph, BL::Context &b_context);

  template <typename T>
  float getRendererPercentDone(T &renderer);

protected:
  BL::RenderEngine b_engine;
  std::string delegateId;
  pxr::HdRenderSettingsMap renderSettings;
  pxr::UsdStageRefPtr stage;
};

class FinalEngine : public Engine {
public:
  using Engine::Engine;
  void sync(BL::Depsgraph &b_depsgraph, BL::Context &b_context, pxr::HdRenderSettingsMap &renderSettings) override;
  void render(BL::Depsgraph &b_depsgraph);

private:
  void renderGL(BL::Depsgraph &b_depsgraph);
  void renderLite(BL::Depsgraph &b_depsgraph);
  void getResolution(BL::RenderSettings b_render, int &width, int &height);
  void updateRenderResult(std::map<std::string, std::vector<float>> &render_images, const std::string &layerName, int width, int height);
  void notifyStatus(float progress, const std::string &title, const std::string &info);
};

class ViewportEngine : public Engine {
public:
  using Engine::Engine;
  void sync(BL::Depsgraph &b_depsgraph, BL::Context &b_context, pxr::HdRenderSettingsMap &renderSettings) override;
  void viewDraw(BL::Depsgraph &b_depsgraph, BL::Context &b_context);

private:
  void notifyStatus(const std::string &title, const std::string &info, bool redraw);

private:
  std::unique_ptr<pxr::UsdImagingGLEngine> imagingGLEngine;
  pxr::UsdImagingGLRenderParams renderParams;
  std::chrono::time_point<std::chrono::steady_clock> timeBegin;
};

PyObject *addPythonSubmodule_engine(PyObject *mod);

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

}   // namespace usdhydra
