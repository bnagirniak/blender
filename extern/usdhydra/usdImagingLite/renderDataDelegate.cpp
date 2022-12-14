/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <iostream>

#include <pxr/imaging/hd/renderBuffer.h>

#include "renderDataDelegate.h"

namespace usdhydra {

HdRenderDataDelegate::HdRenderDataDelegate(HdRenderIndex* parentIndex, SdfPath const& delegateID)
    : HdSceneDelegate(parentIndex, delegateID)
{
  SdfPath renderTaskId = GetDelegateID().AppendElementString("renderTask");
  GetRenderIndex().InsertTask<HdRenderTask>(this, renderTaskId);
  HdReprSelector reprSelector = HdReprSelector(HdReprTokens->smoothHull);
  HdRprimCollection rprimCollection(HdTokens->geometry, reprSelector, false, TfToken());
  rprimCollection.SetRootPath(SdfPath::AbsoluteRootPath());
  SetParameter(renderTaskId, HdTokens->collection, rprimCollection);
  GetRenderIndex().GetChangeTracker().MarkTaskDirty(renderTaskId, HdChangeTracker::DirtyCollection);

  TfTokenVector renderTags{ HdRenderTagTokens->geometry };
  SetParameter(renderTaskId, HdTokens->renderTags, renderTags);
  GetRenderIndex().GetChangeTracker().MarkTaskDirty(renderTaskId, HdChangeTracker::DirtyRenderTags);

}

bool HdRenderDataDelegate::HasParameter(SdfPath const& id, TfToken const& key) const
{
  ValueCache vCache;
  if (TfMapLookup(_valueCacheMap, id, &vCache) &&
      vCache.count(key) > 0) {
      return true;
  }
  return false;
}

VtValue HdRenderDataDelegate::Get(SdfPath const& id, TfToken const& key)
{
  std::cout << "HdRenderDataDelegate::Get - " << id.GetAsString() << " " << key.GetString() << "\n";
  auto vcache = TfMapLookupPtr(_valueCacheMap, id);
  VtValue ret;
  if (vcache && TfMapLookup(*vcache, key, &ret)) {
      return ret;
  }
  return VtValue();
}

HdRenderBufferDescriptor HdRenderDataDelegate::GetRenderBufferDescriptor(SdfPath const& id)
{
  return GetParameter<HdRenderBufferDescriptor>(id, _tokens->renderBufferDescriptor);
}

TfTokenVector HdRenderDataDelegate::GetTaskRenderTags(SdfPath const& taskId)
{
  if (HasParameter(taskId, _tokens->renderTags)) {
      return GetParameter<TfTokenVector>(taskId, _tokens->renderTags);
  }
  return TfTokenVector();
}

bool HdRenderDataDelegate::IsConverged()
{
  HdTaskSharedPtr renderTask = GetRenderIndex().GetTask(GetDelegateID().AppendElementString("renderTask"));
  return ((HdRenderTask &)*renderTask).IsConverged();
}

void HdRenderDataDelegate::SetRendererAov(TfToken const &aovId, HdAovDescriptor &aovDesc)
{
  HdRenderBufferDescriptor desc(GfVec3i(_renderTaskParams.viewport[2] - _renderTaskParams.viewport[0], _renderTaskParams.viewport[3] - _renderTaskParams.viewport[1], 1),
    aovDesc.format, aovDesc.multiSampled);

  SdfPath renderBufferId = GetDelegateID().AppendElementString("aov_" + aovId.GetString());
  GetRenderIndex().InsertBprim(HdPrimTypeTokens->renderBuffer, this, renderBufferId);
  SetParameter(renderBufferId, _tokens->renderBufferDescriptor, desc);
  GetRenderIndex().GetChangeTracker().MarkBprimDirty(renderBufferId, HdRenderBuffer::DirtyDescription);

  HdRenderPassAovBinding binding;
  binding.aovName = aovId;
  binding.renderBufferId = renderBufferId;
  binding.aovSettings = aovDesc.aovSettings;
  _renderTaskParams.aovBindings.push_back(binding);
}

void HdRenderDataDelegate::GetRendererAov(TfToken const &aovId, void *buf)
{
    SdfPath renderBufferId = GetDelegateID().AppendElementString("aov_" + aovId.GetString());
    HdRenderBuffer *rBuf = static_cast<HdRenderBuffer*>(GetRenderIndex().GetBprim(HdPrimTypeTokens->renderBuffer, renderBufferId));

    void *data = rBuf->Map();
    memcpy(buf, data, rBuf->GetWidth() * rBuf->GetHeight() * HdDataSizeOfFormat(rBuf->GetFormat()));
    rBuf->Unmap();
}

HdTaskSharedPtrVector HdRenderDataDelegate::GetTasks()
{
  SdfPath renderTaskId = GetDelegateID().AppendElementString("renderTask");
  HdTaskSharedPtr renderTask = GetRenderIndex().GetTask(renderTaskId);
  return { renderTask };
}

void HdRenderDataDelegate::SetCameraViewport(SdfPath const & cameraId, int width, int height)
{
  _renderTaskParams.viewport = GfVec4d(0, 0, width, height);
  _renderTaskParams.camera = cameraId;
  
  SdfPath renderTaskId = GetDelegateID().AppendElementString("renderTask");
  SetParameter(renderTaskId, HdTokens->params, _renderTaskParams);
  GetRenderIndex().GetChangeTracker().MarkTaskDirty(renderTaskId, HdChangeTracker::DirtyParams);
}


} // namespace usdhydra
