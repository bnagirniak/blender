/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <iostream>

#include <pxr/imaging/hd/renderBuffer.h>
#include <pxr/imaging/hd/renderDelegate.h>

#include "renderTaskDelegate.h"

namespace usdhydra {

/* HdRenderTask */

HdRenderTask::HdRenderTask(HdSceneDelegate* delegate, SdfPath const& id)
    : HdTask(id)
{
}

HdRenderTask::~HdRenderTask()
{
}

bool HdRenderTask::IsConverged() const
{
    return _pass ? _pass->IsConverged() : true;
}

void HdRenderTask::Sync(HdSceneDelegate* delegate,
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
        HdRenderTaskParams params;

        auto value = delegate->Get(GetId(), HdTokens->params);
        if (TF_VERIFY(value.IsHolding<HdRenderTaskParams>())) {
            params = value.UncheckedGet<HdRenderTaskParams>();
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

void HdRenderTask::Prepare(HdTaskContext* ctx,
                           HdRenderIndex* renderIndex)
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

void HdRenderTask::Execute(HdTaskContext* ctx)
{
    // Bind the render state and render geometry with the rendertags (if any)
    if (_pass) {
        _pass->Execute(_passState, GetRenderTags());
    }
}

TfTokenVector const& HdRenderTask::GetRenderTags() const
{
    return _renderTags;
}

// --------------------------------------------------------------------------- //
// VtValue Requirements
// --------------------------------------------------------------------------- //

std::ostream& operator<<(std::ostream& out, const HdRenderTaskParams& pv)
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

bool operator==(const HdRenderTaskParams& lhs, const HdRenderTaskParams& rhs)
{
    return lhs.aovBindings == rhs.aovBindings &&
           lhs.camera == rhs.camera &&
           lhs.viewport == rhs.viewport;
}

bool operator!=(const HdRenderTaskParams& lhs, const HdRenderTaskParams& rhs)
{
    return !(lhs == rhs);
}

/* HdRenderDataDelegate */

HdRenderDataDelegate::HdRenderDataDelegate(HdRenderIndex* parentIndex, SdfPath const& delegateID)
    : HdSceneDelegate(parentIndex, delegateID)
{
  SdfPath renderTaskId = GetTaskID();
  GetRenderIndex().InsertTask<HdRenderTask>(this, renderTaskId);
  GetRenderIndex().GetChangeTracker().MarkTaskDirty(renderTaskId, HdChangeTracker::DirtyCollection);
  GetRenderIndex().GetChangeTracker().MarkTaskDirty(renderTaskId, HdChangeTracker::DirtyRenderTags);
}

SdfPath HdRenderDataDelegate::GetTaskID() const
{
  return GetDelegateID().AppendElementString("task");
}

VtValue HdRenderDataDelegate::Get(SdfPath const& id, TfToken const& key)
{
  std::cout << "HdRenderDataDelegate::Get - " << id.GetAsString() << " " << key.GetString() << "\n";
  if (key == HdTokens->params) {
    return VtValue(_renderTaskParams);
  }
  if (key == HdTokens->collection) {
    HdRprimCollection rprimCollection(HdTokens->geometry, HdReprSelector(HdReprTokens->smoothHull), false, TfToken());
    rprimCollection.SetRootPath(SdfPath::AbsoluteRootPath());
    return VtValue(rprimCollection);
  }
  return VtValue();
}

HdRenderBufferDescriptor HdRenderDataDelegate::GetRenderBufferDescriptor(SdfPath const& id)
{
  std::cout << "HdRenderDataDelegate::GetRenderBufferDescriptor - " << id.GetAsString() << "\n";

  return aovs[id];
}

TfTokenVector HdRenderDataDelegate::GetTaskRenderTags(SdfPath const& taskId)
{
  std::cout << "HdRenderDataDelegate::GetTaskRenderTags - " << taskId.GetAsString() << "\n";

  return { HdRenderTagTokens->geometry };
}

bool HdRenderDataDelegate::IsConverged()
{
  HdTaskSharedPtr renderTask = GetRenderIndex().GetTask(GetTaskID());
  return ((HdRenderTask &)*renderTask).IsConverged();
}

void HdRenderDataDelegate::SetRendererAov(TfToken const &aovName, HdAovDescriptor &aovDesc)
{
  HdRenderBufferDescriptor desc(GfVec3i(_renderTaskParams.viewport[2] - _renderTaskParams.viewport[0], _renderTaskParams.viewport[3] - _renderTaskParams.viewport[1], 1),
    aovDesc.format, aovDesc.multiSampled);

  SdfPath renderBufferId = GetDelegateID().AppendElementString("aov_" + aovName.GetString());
  GetRenderIndex().InsertBprim(HdPrimTypeTokens->renderBuffer, this, renderBufferId);
  
  aovs[renderBufferId] = desc;
  GetRenderIndex().GetChangeTracker().MarkBprimDirty(renderBufferId, HdRenderBuffer::DirtyDescription);

  HdRenderPassAovBinding binding;
  binding.aovName = aovName;
  binding.renderBufferId = renderBufferId;
  binding.aovSettings = aovDesc.aovSettings;
  _renderTaskParams.aovBindings.push_back(binding);

  GetRenderIndex().GetChangeTracker().MarkTaskDirty(GetTaskID(), HdChangeTracker::DirtyParams);
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
  HdTaskSharedPtr renderTask = GetRenderIndex().GetTask(GetTaskID());
  return { renderTask };
}

void HdRenderDataDelegate::SetCameraViewport(SdfPath const & cameraId, int width, int height)
{
  _renderTaskParams.viewport = GfVec4d(0, 0, width, height);
  _renderTaskParams.camera = cameraId;
  
  GetRenderIndex().GetChangeTracker().MarkTaskDirty(GetTaskID(), HdChangeTracker::DirtyParams);
}


} // namespace usdhydra
