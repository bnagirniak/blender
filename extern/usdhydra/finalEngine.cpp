/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <pxr/pxr.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/base/gf/camera.h>
#include <pxr/imaging/glf/drawTarget.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdLux/domeLight.h>
#include <pxr/usd/usdLux/shapingAPI.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>
#include <pxr/usdImaging/usdAppUtils/camera.h>
#include <pxr/base/plug/plugin.h>
#include <pxr/base/plug/registry.h>

#include "intern/usd_hierarchy_iterator.h"

//#include "BKE_main.h"
//#include "BKE_scene.h"
#include "BKE_context.h"
#include "BKE_blender_version.h"

#include "DEG_depsgraph_query.h"

#include "usdImagingLite/engine.h"
#include "usdImagingLite/renderParams.h"
#include "glog/logging.h"

#include "engine.h"
#include "utils.h"


using namespace pxr;

namespace usdhydra {

void FinalEngine::sync(BL::Depsgraph &b_depsgraph, BL::Context &b_context, pxr::HdRenderSettingsMap &renderSettings)
{
  this->renderSettings = renderSettings;
  stage = UsdStage::CreateInMemory();
  exportScene(b_depsgraph, b_context);
}

void FinalEngine::render(BL::Depsgraph &b_depsgraph)
{
  if (b_engine.bl_use_gpu_context()) {
    renderGL(b_depsgraph);
  }
  else {
    renderLite(b_depsgraph);
  }
}

void FinalEngine::renderGL(BL::Depsgraph &b_depsgraph)
{
  std::unique_ptr<UsdImagingGLEngine> imagingGLEngine = std::make_unique<UsdImagingGLEngine>();

  if (!imagingGLEngine->SetRendererPlugin(TfToken(delegateId))) {
    DLOG(ERROR) << "Error in SetRendererPlugin(" << delegateId << ")";
    return;
  }

  for (auto const& setting : renderSettings) {
    imagingGLEngine->SetRendererSetting(setting.first, setting.second);
  }

  BL::Scene b_scene = b_depsgraph.scene_eval();
  
  int width, height;
  getResolution(b_scene.render(), width, height);

  UsdGeomCamera usdCamera = UsdAppUtilsGetCameraAtPath(stage, SdfPath(TfMakeValidIdentifier(b_scene.camera().data().name())));
  GfCamera gfCamera = usdCamera.GetCamera(UsdTimeCode(b_scene.frame_current()));

  GlfDrawTargetRefPtr drawTarget = GlfDrawTarget::New(GfVec2i(width, height));
  drawTarget->Bind();
  drawTarget->AddAttachment("color", GL_RGBA, GL_FLOAT, GL_RGBA);

  imagingGLEngine->SetRenderViewport(GfVec4d(0, 0, width, height));
  imagingGLEngine->SetRendererAov(HdAovTokens->color);

  imagingGLEngine->SetCameraState(gfCamera.GetFrustum().ComputeViewMatrix(),
                                  gfCamera.GetFrustum().ComputeProjectionMatrix());

  UsdImagingGLRenderParams renderParams;
  renderParams.frame = UsdTimeCode(b_scene.frame_current());
  renderParams.clearColor = GfVec4f(1.0, 1.0, 1.0, 0.0);

  imagingGLEngine->Render(stage->GetPseudoRoot(), renderParams);

  map<string, vector<float>> renderImages{{"Combined", vector<float>(width * height * 4)}};   // 4 - number of channels
  vector<float> &pixels = renderImages["Combined"];

  glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, pixels.data());
  drawTarget->Unbind();
  
  updateRenderResult(renderImages, b_depsgraph.view_layer().name(), width, height);
}

void FinalEngine::renderLite(BL::Depsgraph &b_depsgraph)
{
  std::unique_ptr<UsdImagingLiteEngine> imagingLiteEngine = std::make_unique<UsdImagingLiteEngine>();

  if (!imagingLiteEngine->SetRendererPlugin(TfToken(delegateId))) {
    DLOG(ERROR) << "Error in SetRendererPlugin(" << delegateId << ")";
    return;
  }

  for (auto const& setting : renderSettings) {
    imagingLiteEngine->SetRendererSetting(setting.first, setting.second);
  }

  BL::Scene b_scene = b_depsgraph.scene_eval();

  int width, height;
  getResolution(b_scene.render(), width, height);

  UsdGeomCamera usdCamera = UsdAppUtilsGetCameraAtPath(stage, SdfPath(TfMakeValidIdentifier(b_scene.camera().data().name())));
  GfCamera gfCamera = usdCamera.GetCamera(UsdTimeCode(b_scene.frame_current()));

  imagingLiteEngine->SetCameraState(gfCamera);
  imagingLiteEngine->SetRenderViewport(GfVec4d(0, 0, width, height));
  imagingLiteEngine->SetRendererAov(HdAovTokens->color);

  UsdImagingLiteRenderParams renderParams;
  renderParams.frame = UsdTimeCode(b_scene.frame_current());
  renderParams.clearColor = GfVec4f(1.0, 1.0, 1.0, 0.0);

  chrono::time_point<chrono::steady_clock> timeBegin = chrono::steady_clock::now(), timeCurrent;
  chrono::milliseconds elapsedTime;

  float percentDone = 0.0;
  string layerName = b_depsgraph.view_layer().name();

  map<string, vector<float>> renderImages{{"Combined", vector<float>(width * height * 4)}};   // 4 - number of channels
  vector<float> &pixels = renderImages["Combined"];

  while (true) {
    if (b_engine.test_break()) {
      break;
    }

    imagingLiteEngine->Render(stage->GetPseudoRoot(), renderParams);

    percentDone = getRendererPercentDone(*imagingLiteEngine);
    timeCurrent = chrono::steady_clock::now();
    elapsedTime = chrono::duration_cast<chrono::milliseconds>(timeCurrent - timeBegin);

    notifyStatus(percentDone / 100.0,
      b_scene.name() + ": " + layerName,
      "Render Time: " + format_milliseconds(elapsedTime) + " | Done: " + to_string(int(percentDone)) + "%");

    if (imagingLiteEngine->IsConverged()) {
      break;
    }

    imagingLiteEngine->GetRendererAov(HdAovTokens->color, pixels.data());
    updateRenderResult(renderImages, layerName, width, height);
  }

  imagingLiteEngine->GetRendererAov(HdAovTokens->color, pixels.data());
  updateRenderResult(renderImages, layerName, width, height);
}

void FinalEngine::getResolution(BL::RenderSettings b_render, int &width, int &height)
{
  float border_w = 1.0, border_h = 1.0;
  if (b_render.use_border()) {
    border_w = b_render.border_max_x() - b_render.border_min_x();
    border_h = b_render.border_max_y() - b_render.border_min_y();
  }

  width = int(b_render.resolution_x() * border_w * b_render.resolution_percentage() / 100);
  height = int(b_render.resolution_y() * border_h * b_render.resolution_percentage() / 100);
}

void FinalEngine::updateRenderResult(map<string, vector<float>>& renderImages, const string &layerName, int width, int height)
{
  BL::RenderResult b_result = b_engine.begin_result(0, 0, width, height, layerName.c_str(), NULL);
  BL::CollectionRef b_passes = b_result.layers[0].passes;

  for (BL::RenderPass b_pass : b_passes) {
    auto it_image = renderImages.find(b_pass.name());
    if (it_image == renderImages.end()) {
      continue;
    }
    b_pass.rect(it_image->second.data());
  }
  b_engine.end_result(b_result, false, false, false);
}

void FinalEngine::notifyStatus(float progress, const string &title, const string &info)
{
  b_engine.update_progress(progress);
  b_engine.update_stats(title.c_str(), info.c_str());
}

}   // namespace usdhydra
