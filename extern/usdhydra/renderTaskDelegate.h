/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <pxr/imaging/hdx/renderSetupTask.h>
#include <pxr/imaging/hd/sceneDelegate.h>

using namespace pxr;

namespace usdhydra {

class RenderTaskDelegate : public HdSceneDelegate
{
public:
  RenderTaskDelegate(HdRenderIndex* parentIndex, SdfPath const& delegateID);
  ~RenderTaskDelegate() override = default;

  SdfPath GetTaskID() const;

  VtValue Get(SdfPath const& id, TfToken const& key) override;
  HdRenderBufferDescriptor GetRenderBufferDescriptor(SdfPath const& id) override;
  TfTokenVector GetTaskRenderTags(SdfPath const& taskId) override;

  bool IsConverged();
  void SetRendererAov(TfToken const &aovId, HdAovDescriptor &aovDesc);
  void GetRendererAov(TfToken const &id, void *buf);

  HdTaskSharedPtrVector GetTasks();
  void SetCameraAndViewport(SdfPath const& cameraId, int width, int height);

private:
  HdxRenderTaskParams taskParams;
  TfHashMap<SdfPath, HdRenderBufferDescriptor, SdfPath::Hash> bufferDescriptors;
};

} // namespace usdhydra
