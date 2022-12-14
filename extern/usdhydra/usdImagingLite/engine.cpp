/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <iostream>

#include <pxr/imaging/hd/engine.h>
#include <pxr/imaging/hd/camera.h>
#include <pxr/imaging/hd/renderPass.h>
#include <pxr/imaging/hd/renderBuffer.h>
#include <pxr/imaging/hd/rprimCollection.h>
#include <pxr/imaging/hd/rendererPlugin.h>
#include <pxr/imaging/hd/rendererPluginRegistry.h>

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/metrics.h>

#include <pxr/base/work/loops.h>

#include <pxr/base/gf/rotation.h>
#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/frustum.h>

#include "renderTask.h"
#include "engine.h"
#include "renderDataDelegate.h"
#include "../sceneDelegate/blenderSceneDelegate.h"

namespace usdhydra {

class UsdImagingDelegate2 : public UsdImagingDelegate
{
public:
  UsdImagingDelegate2(HdRenderIndex* renderIndex, SdfPath const &delegateId)
    : UsdImagingDelegate(renderIndex, delegateId)
  {}
  ~UsdImagingDelegate2() override = default;
  VtValue Get(SdfPath const& id, TfToken const& key) override
  {
    std::cout << "Get: " << id.GetAsString() << " [" << key.GetString() << "]\n";
    return UsdImagingDelegate::Get(id, key);
  }
  GfMatrix4d GetTransform(SdfPath const& id) override
  {
    std::cout << "GetTransform: " << id.GetAsString() << "\n";
    return UsdImagingDelegate::GetTransform(id);
  }
  HdMeshTopology GetMeshTopology(SdfPath const& id) override
  {
    std::cout << "GetMeshTopology: " << id.GetAsString() << "\n";
    return UsdImagingDelegate::GetMeshTopology(id);
  }
  VtValue GetLightParamValue(SdfPath const& id, TfToken const& key) override
  {
    std::cout << "GetLightParamValue: " << id.GetAsString() << " [" << key.GetString() << "]\n";
    return UsdImagingDelegate::GetLightParamValue(id, key);
  }
  VtValue GetCameraParamValue(SdfPath const& id, TfToken const& key) override
  {
    std::cout << "GetCameraParamValue: " << id.GetAsString() << " [" << key.GetString() << "]\n";
    return UsdImagingDelegate::GetCameraParamValue(id, key);
  }
};


UsdImagingLiteEngine::UsdImagingLiteEngine()
    : _isPopulated(false)
{
}

UsdImagingLiteEngine::~UsdImagingLiteEngine()
{
    _DeleteHydraResources();
}

bool UsdImagingLiteEngine::SetRendererAov(TfToken const &id)
{
    TF_VERIFY(_renderIndex);
    TF_VERIFY(_renderIndex->IsBprimTypeSupported(HdPrimTypeTokens->renderBuffer));

    HdAovDescriptor aovDesc = _renderDelegate->GetDefaultAovDescriptor(id);
    if (aovDesc.format == HdFormatInvalid) {
        TF_RUNTIME_ERROR("Could not set \"%s\" AOV: unsupported by render delegate\n", id.GetText());
        return false;
    }

    _renderDataDelegate->SetRendererAov(id, aovDesc);

    return true;
}

bool UsdImagingLiteEngine::GetRendererAov(TfToken const &id, void *buf)
{
  _renderDataDelegate->GetRendererAov(id, buf);
  return true;
}

SdfPath UsdImagingLiteEngine::_GetRendererAovPath(TfToken const &aov) const
{
    return _renderDataDelegate->GetDelegateID().AppendElementString("aov_" + aov.GetString());
}

VtValue UsdImagingLiteEngine::GetRendererSetting(TfToken const& id) const
{
    TF_VERIFY(_renderDelegate);
    return _renderDelegate->GetRenderSetting(id);
}

void UsdImagingLiteEngine::SetRendererSetting(TfToken const& id, VtValue const& value)
{
    TF_VERIFY(_renderDelegate);
    _renderDelegate->SetRenderSetting(id, value);
}

void UsdImagingLiteEngine::Render(const UsdImagingLiteRenderParams &params)
{
    TF_VERIFY(_sceneDelegate);

    if (!_isPopulated) {
        _sceneDelegate->Sync(nullptr);
        _isPopulated = true;
    }

    //// SetTime will only react if time actually changes.
    //_sceneDelegate->SetTime(params.frame);

    SdfPath renderTaskId = _renderDataDelegate->GetDelegateID().AppendElementString("renderTask");

    SdfPath renderBufferId = _GetRendererAovPath(HdAovTokens->color);

    //for (size_t i = 0; i < _renderDataDelegate->_renderTaskParams.aovBindings.size(); ++i) {
    //    if (_renderDataDelegate->_renderTaskParams.aovBindings[i].renderBufferId == renderBufferId) {
    //        _renderDataDelegate->_renderTaskParams.aovBindings[i].clearValue = params.clearColor;
    //        break;
    //    }
    //}

    _renderDataDelegate->SetParameter(renderTaskId, HdTokens->params, _renderDataDelegate->_renderTaskParams);
    _renderIndex->GetChangeTracker().MarkTaskDirty(renderTaskId, HdChangeTracker::DirtyParams);

    HdTaskSharedPtrVector tasks = _renderDataDelegate->GetTasks();
    {
        // Release the GIL before calling into hydra, in case any hydra plugins
        // call into python.
        TF_PY_ALLOW_THREADS_IN_SCOPE();
        _engine->Execute(_renderIndex.get(), &tasks);
    }
}

bool UsdImagingLiteEngine::IsConverged()
{
    return _renderDataDelegate->IsConverged();
}

void UsdImagingLiteEngine::SetRenderViewport(GfVec4d const & viewport)
{
    _renderDataDelegate->_renderTaskParams.viewport = viewport;
}

void UsdImagingLiteEngine::SetCameraState(const GfCamera& cam)
{
    TF_VERIFY(_renderIndex);
    _freeCameraDelegate->SetCamera(cam);
    _renderDataDelegate->_renderTaskParams.camera = _freeCameraDelegate->GetCameraId();
 }

TfTokenVector UsdImagingLiteEngine::GetRendererPlugins()
{
    HfPluginDescVector pluginDescriptors;
    HdRendererPluginRegistry::GetInstance().GetPluginDescs(&pluginDescriptors);

    TfTokenVector pluginsIds;
    for (auto &descr : pluginDescriptors) {
        pluginsIds.push_back(descr.id);
    }
    return pluginsIds;
}

std::string UsdImagingLiteEngine::GetRendererDisplayName(TfToken const & id)
{
    HfPluginDesc pluginDescriptor;
    if (!TF_VERIFY(HdRendererPluginRegistry::GetInstance().GetPluginDesc(id, &pluginDescriptor))) {
        return "";
    }
    return pluginDescriptor.displayName;
}

bool UsdImagingLiteEngine::SetRendererPlugin(TfToken const & id, BL::Depsgraph &b_depsgraph)
{
    HdRendererPluginRegistry& registry = HdRendererPluginRegistry::GetInstance();

    // Special case: id = TfToken() selects the first plugin in the list.
    const TfToken resolvedId = id.IsEmpty() ? registry.GetDefaultPluginId() : id;

    if (_renderDelegate && _renderDelegate.GetPluginId() == resolvedId) {
        return true;
    }

    TF_PY_ALLOW_THREADS_IN_SCOPE();

    HdPluginRenderDelegateUniqueHandle renderDelegate = registry.CreateRenderDelegate(resolvedId);
    if (!renderDelegate) {
        return false;
    }

    //const GfMatrix4d rootTransform = _sceneDelegate ? _sceneDelegate->GetRootTransform() : GfMatrix4d(1.0);
    //const bool isVisible = _sceneDelegate ? _sceneDelegate->GetRootVisibility() : true;

    _DeleteHydraResources();

    _isPopulated = false;

    // Use the new render delegate.
    _renderDelegate = std::move(renderDelegate);

    // Recreate the render index
    _renderIndex.reset(HdRenderIndex::New(_renderDelegate.Get(), {}));

    // Create the new delegate
    //_sceneDelegate = std::make_unique<UsdImagingDelegate>(_renderIndex.get(), 
    //    SdfPath::AbsoluteRootPath().AppendElementString("usdImagingDelegate"));
    _sceneDelegate = std::make_unique<BlenderSceneDelegate>(_renderIndex.get(), 
        SdfPath::AbsoluteRootPath().AppendElementString("blenderScene"), b_depsgraph);

    _renderDataDelegate = std::make_unique<HdRenderDataDelegate>(_renderIndex.get(),
        SdfPath::AbsoluteRootPath().AppendElementString("renderDataDelegate"));
    _freeCameraDelegate = std::make_unique<HdxFreeCameraSceneDelegate>(_renderIndex.get(),
        SdfPath::AbsoluteRootPath().AppendElementString("freeCamera"));

    // The task context holds on to resources in the render
    // deletegate, so we want to destroy it first and thus
    // create it last.
    _engine = std::make_unique<HdEngine>();

    //// Rebuild state in the new delegate/task controller.
    //_sceneDelegate->SetRootVisibility(isVisible);
    //_sceneDelegate->SetRootTransform(rootTransform);

    return true;
}

bool UsdImagingLiteEngine::IsPauseRendererSupported() const
{
    TF_VERIFY(_renderDelegate);
    return _renderDelegate->IsPauseSupported();
}

bool UsdImagingLiteEngine::PauseRenderer()
{
    TF_PY_ALLOW_THREADS_IN_SCOPE();

    TF_VERIFY(_renderDelegate);
    return _renderDelegate->Pause();
}

bool UsdImagingLiteEngine::ResumeRenderer()
{
    TF_PY_ALLOW_THREADS_IN_SCOPE();

    TF_VERIFY(_renderDelegate);
    return _renderDelegate->Resume();
}

bool UsdImagingLiteEngine::StopRenderer()
{
    TF_PY_ALLOW_THREADS_IN_SCOPE();

    TF_VERIFY(_renderDelegate);
    return _renderDelegate->Stop();
}

bool UsdImagingLiteEngine::RestartRenderer()
{
    TF_PY_ALLOW_THREADS_IN_SCOPE();

    TF_VERIFY(_renderDelegate);
    return _renderDelegate->Restart();
}

void UsdImagingLiteEngine::_DeleteHydraResources()
{
    // Destroy objects in opposite order of construction.
    _engine = nullptr;
    _renderDataDelegate = nullptr;
    _sceneDelegate = nullptr;
    _freeCameraDelegate = nullptr;
    _renderIndex = nullptr;
    _renderDelegate = nullptr;
}

//----------------------------------------------------------------------------
// Resource Information
//----------------------------------------------------------------------------

VtDictionary UsdImagingLiteEngine::GetRenderStats() const
{
    return _renderDelegate->GetRenderStats();
}

} // namespace usdhydra
