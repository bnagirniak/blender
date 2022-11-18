/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <chrono>

#include <Python.h>

#include <pxr/usd/usd/stage.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usdImaging/usdAppUtils/camera.h>

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

#include "usd.h"
#include "intern/usd_exporter_context.h"
#include "BKE_main.h"
#include "BKE_scene.h"
#include "BKE_context.h"
#include "BKE_blender_version.h"

#include "DEG_depsgraph.h"
#include "DEG_depsgraph_query.h"
#include "utils.h"

#include "engine.h"
#include "stage.h"
#include "view_settings.h"

namespace usdhydra {

class Engine {
public:
  Engine(BL::RenderEngine &b_engine, const char* delegateId);
  virtual ~Engine();

  virtual void sync(BL::Depsgraph &b_depsgraph, BL::Context &b_context, pxr::HdRenderSettingsMap &renderSettings) = 0;

protected:
  void exportScene(BL::Depsgraph &b_depsgraph, BL::Context &b_context);

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
  void updateRenderResult(map<string, vector<float>> &render_images, const string &layerName, int width, int height);
  void notifyStatus(float progress, const std::string &title, const std::string &info);
};

class ViewportEngine : public Engine {
public:
  using Engine::Engine;
  void sync(BL::Depsgraph &b_depsgraph, BL::Context &b_context, pxr::HdRenderSettingsMap &renderSettings) override;
  void viewDraw(BL::Depsgraph &b_depsgraph, BL::Context &b_context);

private:
  void notifyStatus(const string &title, const string &info, bool redraw);

private:
  std::unique_ptr<pxr::UsdImagingGLEngine> imagingGLEngine;
  pxr::UsdImagingGLRenderParams renderParams;
  chrono::time_point<chrono::steady_clock> timeBegin;
};

PyObject *addPythonSubmodule_engine(PyObject *mod);

}   // namespace usdhydra
