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

} // namespace usdhydra
