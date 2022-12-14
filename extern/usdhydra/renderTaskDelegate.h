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

class HdRenderTask : public HdTask
{
public:
    HdRenderTask(HdSceneDelegate* delegate, SdfPath const& id);
    ~HdRenderTask() override;

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

class HdRenderDataDelegate : public HdSceneDelegate {
public:
    HdRenderDataDelegate(HdRenderIndex* parentIndex, SdfPath const& delegateID);
    ~HdRenderDataDelegate() override = default;

    SdfPath GetTaskID() const;

    template <typename T>
    void SetParameter(SdfPath const& id, TfToken const& key, T const& value)
    {
        _valueCacheMap[id][key] = value;
    }

    template <typename T>
    T const& GetParameter(SdfPath const& id, TfToken const& key) const
    {
        VtValue vParams;
        ValueCache vCache;
        TF_VERIFY(
            TfMapLookup(_valueCacheMap, id, &vCache) &&
            TfMapLookup(vCache, key, &vParams) &&
            vParams.IsHolding<T>());
        return vParams.Get<T>();
    }

    bool HasParameter(SdfPath const& id, TfToken const& key) const;
    VtValue Get(SdfPath const& id, TfToken const& key) override;
    HdRenderBufferDescriptor GetRenderBufferDescriptor(SdfPath const& id) override;
    TfTokenVector GetTaskRenderTags(SdfPath const& taskId) override;

    bool IsConverged();
    void SetRendererAov(TfToken const &aovId, HdAovDescriptor &aovDesc);
    void GetRendererAov(TfToken const &id, void *buf);

    HdTaskSharedPtrVector GetTasks();
    void SetCameraViewport(SdfPath const& cameraId, int width, int height);

private:
    typedef TfHashMap<TfToken, VtValue, TfToken::HashFunctor> ValueCache;
    typedef TfHashMap<SdfPath, ValueCache, SdfPath::Hash> ValueCacheMap;
    ValueCacheMap _valueCacheMap;
    HdRenderTaskParams _renderTaskParams;
};

} // namespace usdhydra
