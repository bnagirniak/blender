/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <memory>

#include <pxr/imaging/hd/engine.h>
#include <pxr/imaging/hdx/freeCameraSceneDelegate.h>
#include <pxr/imaging/glf/drawTarget.h>
#include <pxr/usdImaging/usdAppUtils/camera.h>

#include "glog/logging.h"

#include "finalEngineGL.h"
#include "utils.h"
#include "sceneDelegate/scene.h"

using namespace std;
using namespace pxr;

namespace usdhydra {

void FinalEngineGL::render(BL::Depsgraph& b_depsgraph)
{
  sceneDelegate = std::make_unique<BlenderSceneDelegate>(renderIndex.get(), 
    SdfPath::AbsoluteRootPath().AppendElementString("blenderScene"), b_depsgraph);
  sceneDelegate->Populate();

  for (auto const& setting : renderSettings) {
    renderDelegate->SetRenderSetting(setting.first, setting.second);
  }

  SceneExport sceneExport(b_depsgraph);
  auto resolution = sceneExport.resolution();
  int width = resolution.first, height = resolution.second;

  GfCamera gfCamera = sceneExport.gfCamera();
  freeCameraDelegate->SetCamera(gfCamera);
  renderTaskDelegate->SetCameraAndViewport(freeCameraDelegate->GetCameraId(), GfVec4d(0, 0, width, height));
  
  HdTaskSharedPtrVector tasks = renderTaskDelegate->GetTasks();

  chrono::time_point<chrono::steady_clock> timeBegin = chrono::steady_clock::now(), timeCurrent;
  chrono::milliseconds elapsedTime;

  float percentDone = 0.0;
  string sceneName = sceneExport.sceneName(), layerName = sceneExport.layerName();

  map<string, vector<float>> renderImages{{"Combined", vector<float>(width * height * 4)}};   // 4 - number of channels
  vector<float> &pixels = renderImages["Combined"];

  GLuint FramebufferName = 0;
  glGenFramebuffers(1, &FramebufferName);
  glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

  // The texture we're going to render to
  GLuint renderedTexture;
  glGenTextures(1, &renderedTexture);

  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture(GL_TEXTURE_2D, renderedTexture);

  // Give an empty image to OpenGL ( the last "0" )
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);

  // Poor filtering. Needed !
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // Set "renderedTexture" as our colour attachement #0
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

  // Generate vertex array
  GLuint VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  {
    // Release the GIL before calling into hydra, in case any hydra plugins call into python.
    TF_PY_ALLOW_THREADS_IN_SCOPE();
    engine->Execute(renderIndex.get(), &tasks);
  }

  while (true) {
    if (b_engine.test_break()) {
      break;
    }

    percentDone = getRendererPercentDone();
    timeCurrent = chrono::steady_clock::now();
    elapsedTime = chrono::duration_cast<chrono::milliseconds>(timeCurrent - timeBegin);

    notifyStatus(percentDone / 100.0, sceneName + ": " + layerName,
      "Render Time: " + formatDuration(elapsedTime) + " | Done: " + to_string(int(percentDone)) + "%");

    if (renderTaskDelegate->IsConverged()) {
      break;
    }

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels.data());
    updateRenderResult(renderImages, layerName, width, height);
  }

  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels.data());
  updateRenderResult(renderImages, layerName, width, height);
}

}   // namespace usdhydra
