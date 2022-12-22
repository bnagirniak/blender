/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <chrono>

#include <epoxy/gl.h>

#include "engine.h"

namespace usdhydra {

class GLTexture
{
public:
  GLTexture();

private:
  GLuint textureId;
  int width, height, channels;
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

  //GLuint texture1;
};

}   // namespace usdhydra
