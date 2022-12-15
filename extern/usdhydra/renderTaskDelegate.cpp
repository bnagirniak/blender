/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <iostream>

#include <pxr/imaging/hd/renderBuffer.h>
#include <pxr/imaging/hd/renderDelegate.h>

#include "renderTaskDelegate.h"

namespace usdhydra {

/* RenderTask */

RenderTask::RenderTask(HdSceneDelegate* delegate, SdfPath const& id)
  : HdTask(id)
{
}

RenderTask::~RenderTask()
{
}

bool RenderTask::IsConverged() const
{
    return _pass ? _pass->IsConverged() : true;
}

void RenderTask::Sync(HdSceneDelegate* delegate,
                      HdTaskContext* ctx,
                      HdDirtyBits* dirtyBits)
{
    auto renderIndex = &delegate->GetRenderIndex();

    if ((*dirtyBits) & HdChangeTracker::DirtyCollection) {
        VtValue val = delegate->Get(GetId(), HdTokens->collection);
        auto collection = val.Get<HdRprimCollection>();

        // Check for cases where the collection is empty (i.e. default
        // constructed).  To do this, the code looks at the root paths,
        // if it is empty, the collection doesn't refer to any prims at
        // all.
        if (collection.GetName().IsEmpty()) {
            _pass.reset();
        } else {
            if (!_pass) {
                auto renderDelegate = renderIndex->GetRenderDelegate();
                _pass = renderDelegate->CreateRenderPass(renderIndex, collection);
            } else {
                _pass->SetRprimCollection(collection);
            }
        }
    }

    if ((*dirtyBits) & HdChangeTracker::DirtyParams) {
        RenderTaskParams params;

        auto value = delegate->Get(GetId(), HdTokens->params);
        if (TF_VERIFY(value.IsHolding<RenderTaskParams>())) {
            params = value.UncheckedGet<RenderTaskParams>();
        }

        _aovBindings = params.aovBindings;
        _viewport = params.viewport;
        _cameraId = params.camera;
    }

    if ((*dirtyBits) & HdChangeTracker::DirtyRenderTags) {
        _renderTags = _GetTaskRenderTags(delegate);
    }

    if (_pass) {
        _pass->Sync();
    }

    *dirtyBits = HdChangeTracker::Clean;
}

void RenderTask::Prepare(HdTaskContext* ctx, HdRenderIndex* renderIndex)
{
    if (!_passState) {
        _passState = renderIndex->GetRenderDelegate()->CreateRenderPassState();
    }

    // Prepare AOVS
    {
        // Walk the aov bindings, resolving the render index references as they're
        // encountered.
        for (size_t i = 0; i < _aovBindings.size(); ++i) {
            if (_aovBindings[i].renderBuffer == nullptr) {
                _aovBindings[i].renderBuffer = static_cast<HdRenderBuffer*>(renderIndex->GetBprim(HdPrimTypeTokens->renderBuffer, _aovBindings[i].renderBufferId));
            }
        }
        _passState->SetAovBindings(_aovBindings);

        // XXX Tasks that are not RenderTasks (OIT, ColorCorrection etc) also need
        // access to AOVs, but cannot access SetupTask or RenderPassState.
        //(*ctx)[HdxTokens->aovBindings] = VtValue(_aovBindings);
    }

    // Prepare Camera
    {
        auto camera = static_cast<const HdCamera*>(renderIndex->GetSprim(HdPrimTypeTokens->camera, _cameraId));
        TF_VERIFY(camera);
        _passState->SetCameraAndViewport(camera, _viewport);
    }

    _passState->Prepare(renderIndex->GetResourceRegistry());
}

void RenderTask::Execute(HdTaskContext* ctx)
{
    // Bind the render state and render geometry with the rendertags (if any)
    if (_pass) {
        _pass->Execute(_passState, GetRenderTags());
    }
}

TfTokenVector const& RenderTask::GetRenderTags() const
{
    return _renderTags;
}

// --------------------------------------------------------------------------- //
// VtValue Requirements
// --------------------------------------------------------------------------- //

std::ostream& operator<<(std::ostream& out, const RenderTaskParams& pv)
{
    out << "RenderTask Params:\n";
    out << "camera: " << pv.camera << '\n';
    out << "viewport: " << pv.viewport << '\n';
    out << "aovBindings: ";
    for (auto const& a : pv.aovBindings) {
        out << a << " ";
    }
    out << '\n';
    return out;
}

bool operator==(const RenderTaskParams& lhs, const RenderTaskParams& rhs)
{
    return lhs.aovBindings == rhs.aovBindings &&
           lhs.camera == rhs.camera &&
           lhs.viewport == rhs.viewport;
}

bool operator!=(const RenderTaskParams& lhs, const RenderTaskParams& rhs)
{
    return !(lhs == rhs);
}

/* RenderTaskDelegate */

RenderTaskDelegate::RenderTaskDelegate(HdRenderIndex* parentIndex, SdfPath const& delegateID)
    : HdSceneDelegate(parentIndex, delegateID)
{
  SdfPath renderTaskId = GetTaskID();
  GetRenderIndex().InsertTask<RenderTask>(this, renderTaskId);
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
  return ((RenderTask &)*renderTask).IsConverged();
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

void RenderTaskDelegate::SetCameraViewport(SdfPath const & cameraId, int width, int height)
{
  taskParams.viewport = GfVec4d(0, 0, width, height);
  taskParams.camera = cameraId;
  
  GetRenderIndex().GetChangeTracker().MarkTaskDirty(GetTaskID(), HdChangeTracker::DirtyParams);
}


} // namespace usdhydra
