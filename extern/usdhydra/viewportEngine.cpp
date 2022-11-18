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

#include "view_settings.h"

#include "engine.h"
#include "utils.h"


using namespace pxr;

namespace usdhydra {

void ViewportEngine::sync(BL::Depsgraph &b_depsgraph, BL::Context &b_context, pxr::HdRenderSettingsMap &renderSettings_)
{
  renderSettings = renderSettings_;
  if (!imagingGLEngine) {
    stage = UsdStage::CreateInMemory();
    exportScene(b_depsgraph, b_context);

    imagingGLEngine = std::make_unique<UsdImagingGLEngine>();
    imagingGLEngine->SetRendererPlugin(TfToken(delegateId));
  }

  for (auto const& pair : renderSettings) {
    imagingGLEngine->SetRendererSetting(pair.first, pair.second);
  }
}

void ViewportEngine::viewDraw(BL::Depsgraph &b_depsgraph, BL::Context &b_context)
{
  ViewSettings viewSettings(b_context);
  if (viewSettings.get_width() * viewSettings.get_height() == 0) {
    return;
  };

  BL::Scene b_scene = b_depsgraph.scene_eval();
  GfCamera gfCamera = viewSettings.export_camera();

  vector<GfVec4f> clipPlanes = gfCamera.GetClippingPlanes();

  for (int i = 0; i < clipPlanes.size(); i++) {
    renderParams.clipPlanes.push_back((GfVec4d)clipPlanes[i]);
  }

  imagingGLEngine->SetCameraState(gfCamera.GetFrustum().ComputeViewMatrix(),
                                  gfCamera.GetFrustum().ComputeProjectionMatrix());
  imagingGLEngine->SetRenderViewport(GfVec4d((double)viewSettings.border[0][0], (double)viewSettings.border[0][1],
                                             (double)viewSettings.border[1][0], (double)viewSettings.border[1][1]));

  b_engine.bind_display_space_shader(b_scene);

  if (getRendererPercentDone(*imagingGLEngine) == 0.0f) {
    timeBegin = chrono::steady_clock::now();
  }

  imagingGLEngine->Render(stage->GetPseudoRoot(), renderParams);

  b_engine.unbind_display_space_shader();

  glClear(GL_DEPTH_BUFFER_BIT);

  chrono::time_point<chrono::steady_clock> timeCurrent = chrono::steady_clock::now();
  chrono::milliseconds elapsedTime = chrono::duration_cast<chrono::milliseconds>(timeCurrent - timeBegin);

  string formattedTime = format_milliseconds(elapsedTime);

  if (!imagingGLEngine->IsConverged()) {
    notifyStatus("Time: " + formattedTime + " | Done: " + to_string(int(getRendererPercentDone(*imagingGLEngine))) + "%",
                 "Render", true);
  }
  else {
    notifyStatus(("Time: " + formattedTime).c_str(), "Rendering Done", false);
  }
}

void ViewportEngine::notifyStatus(const string &info, const string &status, bool redraw)
{
  b_engine.update_stats(status.c_str(), info.c_str());

  if (redraw) {
    b_engine.tag_redraw();
  }
}

}   // namespace usdhydra
