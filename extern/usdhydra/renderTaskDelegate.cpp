/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <iostream>

#include <pxr/imaging/hd/renderBuffer.h>
#include <pxr/imaging/hd/renderDelegate.h>
#include <pxr/imaging/hdx/renderTask.h>

#include "renderTaskDelegate.h"

namespace usdhydra {

RenderTaskDelegate::RenderTaskDelegate(HdRenderIndex* parentIndex, SdfPath const& delegateID)
    : HdSceneDelegate(parentIndex, delegateID)
{
  SdfPath renderTaskId = GetTaskID();
  GetRenderIndex().InsertTask<HdxRenderTask>(this, renderTaskId);
  GetRenderIndex().GetChangeTracker().MarkTaskDirty(renderTaskId, HdChangeTracker::DirtyCollection);
  GetRenderIndex().GetChangeTracker().MarkTaskDirty(renderTaskId, HdChangeTracker::DirtyRenderTags);
}

SdfPath RenderTaskDelegate::GetTaskID() const
{
  return GetDelegateID().AppendElementString("task");
}

VtValue RenderTaskDelegate::Get(SdfPath const& id, TfToken const& key)
{
  std::cout << "RenderTaskDelegate::Get - " << id.GetAsString() << " " << key.GetString() << "\n";
  if (key == HdTokens->params) {
    return VtValue(taskParams);
  }
  if (key == HdTokens->collection) {
    HdRprimCollection rprimCollection(HdTokens->geometry, HdReprSelector(HdReprTokens->smoothHull), false, TfToken());
    rprimCollection.SetRootPath(SdfPath::AbsoluteRootPath());
    return VtValue(rprimCollection);
  }
  return VtValue();
}

HdRenderBufferDescriptor RenderTaskDelegate::GetRenderBufferDescriptor(SdfPath const& id)
{
  std::cout << "RenderTaskDelegate::GetRenderBufferDescriptor - " << id.GetAsString() << "\n";

  return bufferDescriptors[id];
}

TfTokenVector RenderTaskDelegate::GetTaskRenderTags(SdfPath const& taskId)
{
  std::cout << "RenderTaskDelegate::GetTaskRenderTags - " << taskId.GetAsString() << "\n";

  return { HdRenderTagTokens->geometry };
}

bool RenderTaskDelegate::IsConverged()
{
  HdTaskSharedPtr renderTask = GetRenderIndex().GetTask(GetTaskID());
  return ((HdxRenderTask &)*renderTask).IsConverged();
}

void RenderTaskDelegate::SetRendererAov(TfToken const &aovName, HdAovDescriptor &aovDesc)
{
  HdRenderBufferDescriptor desc(GfVec3i(taskParams.viewport[2] - taskParams.viewport[0], taskParams.viewport[3] - taskParams.viewport[1], 1),
    aovDesc.format, aovDesc.multiSampled);

  SdfPath renderBufferId = GetDelegateID().AppendElementString("aov_" + aovName.GetString());
  GetRenderIndex().InsertBprim(HdPrimTypeTokens->renderBuffer, this, renderBufferId);
  
  bufferDescriptors[renderBufferId] = desc;
  GetRenderIndex().GetChangeTracker().MarkBprimDirty(renderBufferId, HdRenderBuffer::DirtyDescription);

  HdRenderPassAovBinding binding;
  binding.aovName = aovName;
  binding.renderBufferId = renderBufferId;
  binding.aovSettings = aovDesc.aovSettings;
  taskParams.aovBindings.push_back(binding);

  GetRenderIndex().GetChangeTracker().MarkTaskDirty(GetTaskID(), HdChangeTracker::DirtyParams);
}

void RenderTaskDelegate::GetRendererAov(TfToken const &aovId, void *buf)
{
    SdfPath renderBufferId = GetDelegateID().AppendElementString("aov_" + aovId.GetString());
    HdRenderBuffer *rBuf = static_cast<HdRenderBuffer*>(GetRenderIndex().GetBprim(HdPrimTypeTokens->renderBuffer, renderBufferId));

    void *data = rBuf->Map();
    memcpy(buf, data, rBuf->GetWidth() * rBuf->GetHeight() * HdDataSizeOfFormat(rBuf->GetFormat()));
    rBuf->Unmap();
}

HdTaskSharedPtrVector RenderTaskDelegate::GetTasks()
{
  HdTaskSharedPtr renderTask = GetRenderIndex().GetTask(GetTaskID());
  return { renderTask };
}

void RenderTaskDelegate::SetCameraAndViewport(SdfPath const & cameraId, int width, int height)
{
  taskParams.viewport = GfVec4d(0, 0, width, height);
  taskParams.camera = cameraId;
  
  GetRenderIndex().GetChangeTracker().MarkTaskDirty(GetTaskID(), HdChangeTracker::DirtyParams);
}


} // namespace usdhydra
