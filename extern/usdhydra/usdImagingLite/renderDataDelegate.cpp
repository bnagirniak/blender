/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include "renderDataDelegate.h"

namespace usdhydra {

HdRenderDataDelegate::HdRenderDataDelegate(HdRenderIndex* parentIndex, SdfPath const& delegateID)
    : HdSceneDelegate(parentIndex, delegateID)
{}

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
    auto vcache = TfMapLookupPtr(_valueCacheMap, id);
    VtValue ret;
    if (vcache && TfMapLookup(*vcache, key, &ret)) {
        return ret;
    }
    TF_CODING_ERROR("%s:%s doesn't exist in the value cache\n",
        id.GetText(), key.GetText());
    return VtValue();
}

GfMatrix4d HdRenderDataDelegate::GetTransform(SdfPath const& id)
{
    // We expect this to be called only for the free cam.
    VtValue val = GetCameraParamValue(id, HdTokens->transform);
    GfMatrix4d xform(1.0);
    if (val.IsHolding<GfMatrix4d>()) {
        xform = val.Get<GfMatrix4d>();//.GetInverse(); // camera to world
    }
    else {
        TF_CODING_ERROR(
            "Unexpected call to GetTransform for %s in HdxTaskController's "
            "internal scene delegate.\n", id.GetText());
    }
    return xform;
}

VtValue HdRenderDataDelegate::GetCameraParamValue(SdfPath const& id, TfToken const& key)
{
    return Get(id, key);
}

VtValue HdRenderDataDelegate::GetLightParamValue(SdfPath const& id, TfToken const& paramName)
{
    return Get(id, paramName);
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

} // namespace usdhydra
