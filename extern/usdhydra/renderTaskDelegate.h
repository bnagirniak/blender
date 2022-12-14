/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <pxr/pxr.h>
#include <pxr/imaging/hd/task.h>
#include <pxr/imaging/hd/renderPass.h>
#include <pxr/imaging/hd/renderPassState.h>

#include <pxr/imaging/hd/camera.h>
#include <pxr/imaging/hd/sceneDelegate.h>
#include <pxr/imaging/hd/renderIndex.h>
#include <pxr/usd/usd/stage.h>

using namespace pxr;

namespace usdhydra {

TF_DEFINE_PRIVATE_TOKENS(_tokens,
    (renderBufferDescriptor)
    (renderTags));

class RenderTask : public HdTask
{
public:
    RenderTask(HdSceneDelegate* delegate, SdfPath const& id);
    ~RenderTask() override;

    bool IsConverged() const;

    /// Sync the render pass resources
    void Sync(HdSceneDelegate* delegate,
              HdTaskContext* ctx,
              HdDirtyBits* dirtyBits) override;

    /// Prepare the tasks resources
    void Prepare(HdTaskContext* ctx,
                 HdRenderIndex* renderIndex) override;

    /// Execute render pass task
    void Execute(HdTaskContext* ctx) override;

    /// Collect Render Tags used by the task.
    TfTokenVector const& GetRenderTags() const override;

private:
    HdRenderPassSharedPtr _pass;
    HdRenderPassStateSharedPtr _passState;

    TfTokenVector _renderTags;
    GfVec4d _viewport;
    SdfPath _cameraId;
    HdRenderPassAovBindingVector _aovBindings;
};

struct HdRenderTaskParams
{
    // Should not be empty.
    HdRenderPassAovBindingVector aovBindings;

    SdfPath camera;
    GfVec4d viewport = GfVec4d(0.0);
};

// VtValue requirements
std::ostream& operator<<(std::ostream& out, const HdRenderTaskParams& pv);
bool operator==(const HdRenderTaskParams& lhs, const HdRenderTaskParams& rhs);
bool operator!=(const HdRenderTaskParams& lhs, const HdRenderTaskParams& rhs);

class RenderTaskDelegate : public HdSceneDelegate {
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
    void SetCameraViewport(SdfPath const& cameraId, int width, int height);

private:
    HdRenderTaskParams _renderTaskParams;
    TfHashMap<SdfPath, HdRenderBufferDescriptor, SdfPath::Hash> aovs;
};

} // namespace usdhydra
