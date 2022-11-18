/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <pxr/pxr.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/base/gf/camera.h>
#include <pxr/imaging/glf/drawTarget.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdLux/domeLight.h>
#include <pxr/usd/usdLux/shapingAPI.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>
#include <pxr/usdImaging/usdAppUtils/camera.h>
#include <pxr/base/plug/plugin.h>
#include <pxr/base/plug/registry.h>

#include "intern/usd_hierarchy_iterator.h"

//#include "BKE_main.h"
//#include "BKE_scene.h"
#include "BKE_context.h"
#include "BKE_blender_version.h"

#include "DEG_depsgraph_query.h"

#include "usdImagingLite/engine.h"
#include "usdImagingLite/renderParams.h"
#include "glog/logging.h"

#include "view_settings.h"

#include "engine.h"
#include "utils.h"


using namespace pxr;

namespace usdhydra {

Engine::Engine(BL::RenderEngine &b_engine, const char* delegateId)
  : b_engine(b_engine)
  , delegateId(delegateId)
{
}

Engine::~Engine()
{
}

void Engine::exportScene(BL::Depsgraph& b_depsgraph, BL::Context& b_context)
{
  Depsgraph *depsgraph = (Depsgraph *)b_depsgraph.ptr.data;

  Scene *scene = DEG_get_input_scene(depsgraph);
  World *world = scene->world;

  DEG_graph_build_for_all_objects(depsgraph);

  bContext *C = (bContext *)b_context.ptr.data;
  Main *bmain = CTX_data_main(C);
  USDExportParams usd_export_params;

  usd_export_params.selected_objects_only = false;
  usd_export_params.visible_objects_only = false;

  //stage->Reload();

  stage->SetMetadata(UsdGeomTokens->upAxis, VtValue(UsdGeomTokens->z));
  stage->SetMetadata(UsdGeomTokens->metersPerUnit, static_cast<double>(scene->unit.scale_length));
  stage->GetRootLayer()->SetDocumentation(std::string("Blender v") + BKE_blender_version_string());

  /* Set up the stage for animated data. */
  //if (data->params.export_animation) {
  //  stage->SetTimeCodesPerSecond(FPS);
  //  stage->SetStartTimeCode(scene->r.sfra);
  //  stage->SetEndTimeCode(scene->r.efra);
  //}

  blender::io::usd::USDHierarchyIterator iter(bmain, depsgraph, stage, usd_export_params);
  iter.iterate_and_write();
  iter.release_writers();
}

void FinalEngine::sync(BL::Depsgraph &b_depsgraph, BL::Context &b_context, pxr::HdRenderSettingsMap &renderSettings)
{
  this->renderSettings = renderSettings;
  stage = UsdStage::CreateInMemory();
  exportScene(b_depsgraph, b_context);
}

void FinalEngine::render(BL::Depsgraph &b_depsgraph)
{
  if (b_engine.bl_use_gpu_context()) {
    renderGL(b_depsgraph);
  }
  else {
    renderLite(b_depsgraph);
  }
}

void FinalEngine::renderGL(BL::Depsgraph &b_depsgraph)
{
  std::unique_ptr<UsdImagingGLEngine> imagingGLEngine = std::make_unique<UsdImagingGLEngine>();

  if (!imagingGLEngine->SetRendererPlugin(TfToken(delegateId))) {
    DLOG(ERROR) << "Error in SetRendererPlugin(" << delegateId << ")";
    return;
  }

  for (auto const& setting : renderSettings) {
    imagingGLEngine->SetRendererSetting(setting.first, setting.second);
  }

  BL::Scene b_scene = b_depsgraph.scene_eval();
  
  int width, height;
  getResolution(b_scene.render(), width, height);

  UsdGeomCamera usdCamera = UsdAppUtilsGetCameraAtPath(stage, SdfPath(TfMakeValidIdentifier(b_scene.camera().data().name())));
  GfCamera gfCamera = usdCamera.GetCamera(UsdTimeCode(b_scene.frame_current()));

  GlfDrawTargetRefPtr drawTarget = GlfDrawTarget::New(GfVec2i(width, height));
  drawTarget->Bind();
  drawTarget->AddAttachment("color", GL_RGBA, GL_FLOAT, GL_RGBA);

  imagingGLEngine->SetRenderViewport(GfVec4d(0, 0, width, height));
  imagingGLEngine->SetRendererAov(HdAovTokens->color);

  imagingGLEngine->SetCameraState(gfCamera.GetFrustum().ComputeViewMatrix(),
                                  gfCamera.GetFrustum().ComputeProjectionMatrix());

  UsdImagingGLRenderParams renderParams;
  renderParams.frame = UsdTimeCode(b_scene.frame_current());
  renderParams.clearColor = GfVec4f(1.0, 1.0, 1.0, 0.0);

  imagingGLEngine->Render(stage->GetPseudoRoot(), renderParams);

  map<string, vector<float>> renderImages{{"Combined", vector<float>(width * height * 4)}};   // 4 - number of channels
  vector<float> &pixels = renderImages["Combined"];

  glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, pixels.data());
  drawTarget->Unbind();
  
  updateRenderResult(renderImages, b_depsgraph.view_layer().name(), width, height);
}

template <typename T>
float getRendererPercentDone(T &renderer)
{
  float percent = 0.0;

  VtDictionary render_stats = renderer.GetRenderStats();
  auto it = render_stats.find("percentDone");
  if (it != render_stats.end()) {
    percent = (float)it->second.UncheckedGet<double>();
  }

  return round(percent * 10.0f) / 10.0f;
}

void FinalEngine::renderLite(BL::Depsgraph &b_depsgraph)
{
  std::unique_ptr<UsdImagingLiteEngine> imagingLiteEngine = std::make_unique<UsdImagingLiteEngine>();

  if (!imagingLiteEngine->SetRendererPlugin(TfToken(delegateId))) {
    DLOG(ERROR) << "Error in SetRendererPlugin(" << delegateId << ")";
    return;
  }

  for (auto const& setting : renderSettings) {
    imagingLiteEngine->SetRendererSetting(setting.first, setting.second);
  }

  BL::Scene b_scene = b_depsgraph.scene_eval();

  int width, height;
  getResolution(b_scene.render(), width, height);

  UsdGeomCamera usdCamera = UsdAppUtilsGetCameraAtPath(stage, SdfPath(TfMakeValidIdentifier(b_scene.camera().data().name())));
  GfCamera gfCamera = usdCamera.GetCamera(UsdTimeCode(b_scene.frame_current()));

  imagingLiteEngine->SetCameraState(gfCamera);
  imagingLiteEngine->SetRenderViewport(GfVec4d(0, 0, width, height));
  imagingLiteEngine->SetRendererAov(HdAovTokens->color);

  UsdImagingLiteRenderParams renderParams;
  renderParams.frame = UsdTimeCode(b_scene.frame_current());
  renderParams.clearColor = GfVec4f(1.0, 1.0, 1.0, 0.0);

  chrono::time_point<chrono::steady_clock> timeBegin = chrono::steady_clock::now(), timeCurrent;
  chrono::milliseconds elapsedTime;

  float percentDone = 0.0;
  string layerName = b_depsgraph.view_layer().name();

  map<string, vector<float>> renderImages{{"Combined", vector<float>(width * height * 4)}};   // 4 - number of channels
  vector<float> &pixels = renderImages["Combined"];

  while (true) {
    if (b_engine.test_break()) {
      break;
    }

    imagingLiteEngine->Render(stage->GetPseudoRoot(), renderParams);

    percentDone = getRendererPercentDone(*imagingLiteEngine);
    timeCurrent = chrono::steady_clock::now();
    elapsedTime = chrono::duration_cast<chrono::milliseconds>(timeCurrent - timeBegin);

    notifyStatus(percentDone / 100.0,
      b_scene.name() + ": " + layerName,
      "Render Time: " + format_milliseconds(elapsedTime) + " | Done: " + to_string(int(percentDone)) + "%");

    if (imagingLiteEngine->IsConverged()) {
      break;
    }

    imagingLiteEngine->GetRendererAov(HdAovTokens->color, pixels.data());
    updateRenderResult(renderImages, layerName, width, height);
  }

  imagingLiteEngine->GetRendererAov(HdAovTokens->color, pixels.data());
  updateRenderResult(renderImages, layerName, width, height);
}

void FinalEngine::getResolution(BL::RenderSettings b_render, int &width, int &height)
{
  float border_w = 1.0, border_h = 1.0;
  if (b_render.use_border()) {
    border_w = b_render.border_max_x() - b_render.border_min_x();
    border_h = b_render.border_max_y() - b_render.border_min_y();
  }

  width = int(b_render.resolution_x() * border_w * b_render.resolution_percentage() / 100);
  height = int(b_render.resolution_y() * border_h * b_render.resolution_percentage() / 100);
}

void FinalEngine::updateRenderResult(map<string, vector<float>>& renderImages, const string &layerName, int width, int height)
{
  BL::RenderResult b_result = b_engine.begin_result(0, 0, width, height, layerName.c_str(), NULL);
  BL::CollectionRef b_passes = b_result.layers[0].passes;

  for (BL::RenderPass b_pass : b_passes) {
    auto it_image = renderImages.find(b_pass.name());
    if (it_image == renderImages.end()) {
      continue;
    }
    b_pass.rect(it_image->second.data());
  }
  b_engine.end_result(b_result, false, false, false);
}

void FinalEngine::notifyStatus(float progress, const string &title, const string &info)
{
  b_engine.update_progress(progress);
  b_engine.update_stats(title.c_str(), info.c_str());
}

void ViewportEngine::sync(BL::Depsgraph &b_depsgraph, BL::Context &b_context, pxr::HdRenderSettingsMap &renderSettings_)
{
  renderSettings = renderSettings_;
  if (!imagingGLEngine) {
    stage = UsdStage::CreateInMemory();
    exportScene(b_depsgraph, b_context);

    imagingGLEngine = std::make_unique<UsdImagingGLEngine>();
    imagingGLEngine->SetRendererPlugin(TfToken(delegateId));
  }

  for (auto const& pair : renderSettings) {
    imagingGLEngine->SetRendererSetting(pair.first, pair.second);
  }
}

void ViewportEngine::viewDraw(BL::Depsgraph &b_depsgraph, BL::Context &b_context)
{
  ViewSettings viewSettings(b_context);
  if (viewSettings.get_width() * viewSettings.get_height() == 0) {
    return;
  };

  BL::Scene b_scene = b_depsgraph.scene_eval();
  GfCamera gfCamera = viewSettings.export_camera();

  vector<GfVec4f> clipPlanes = gfCamera.GetClippingPlanes();

  for (int i = 0; i < clipPlanes.size(); i++) {
    renderParams.clipPlanes.push_back((GfVec4d)clipPlanes[i]);
  }

  imagingGLEngine->SetCameraState(gfCamera.GetFrustum().ComputeViewMatrix(),
                                  gfCamera.GetFrustum().ComputeProjectionMatrix());
  imagingGLEngine->SetRenderViewport(GfVec4d((double)viewSettings.border[0][0], (double)viewSettings.border[0][1],
                                             (double)viewSettings.border[1][0], (double)viewSettings.border[1][1]));

  b_engine.bind_display_space_shader(b_scene);

  if (getRendererPercentDone(*imagingGLEngine) == 0.0f) {
    timeBegin = chrono::steady_clock::now();
  }

  imagingGLEngine->Render(stage->GetPseudoRoot(), renderParams);

  b_engine.unbind_display_space_shader();

  glClear(GL_DEPTH_BUFFER_BIT);

  chrono::time_point<chrono::steady_clock> timeCurrent = chrono::steady_clock::now();
  chrono::milliseconds elapsedTime = chrono::duration_cast<chrono::milliseconds>(timeCurrent - timeBegin);

  string formattedTime = format_milliseconds(elapsedTime);

  if (!imagingGLEngine->IsConverged()) {
    notifyStatus("Time: " + formattedTime + " | Done: " + to_string(int(getRendererPercentDone(*imagingGLEngine))) + "%",
                 "Render", true);
  }
  else {
    notifyStatus(("Time: " + formattedTime).c_str(), "Rendering Done", false);
  }
}

void ViewportEngine::notifyStatus(const string &info, const string &status, bool redraw)
{
  b_engine.update_stats(status.c_str(), info.c_str());

  if (redraw) {
    b_engine.tag_redraw();
  }
}

/* ------------------------------------------------------------------------- */
/* Python API for Engine
 */

static PyObject *create_func(PyObject * /*self*/, PyObject *args)
{
  DLOG(INFO) << "create_func";
  PyObject *b_pyengine;
  char *engineType, *delegateId;
  if (!PyArg_ParseTuple(args, "Oss", &b_pyengine, &engineType, &delegateId)) {
    Py_RETURN_NONE;
  }

  PointerRNA b_engineptr;
  RNA_pointer_create(NULL, &RNA_RenderEngine, (void *)PyLong_AsVoidPtr(b_pyengine), &b_engineptr);
  BL::RenderEngine b_engine(b_engineptr);

  /* create engine */
  Engine *engine;
  if (string(engineType) == "VIEWPORT") {
    engine = new ViewportEngine(b_engine, delegateId);
  }
  else {
    engine = new FinalEngine(b_engine, delegateId);
  }

  return PyLong_FromVoidPtr(engine);
}

static PyObject *free_func(PyObject * /*self*/, PyObject *args)
{
  LOG(INFO) << "free_func";
  PyObject *pyengine;
  if (!PyArg_ParseTuple(args, "O", &pyengine)) {
    Py_RETURN_NONE;
  }

  delete (Engine *)PyLong_AsVoidPtr(pyengine);
  Py_RETURN_NONE;
}

static PyObject *sync_func(PyObject * /*self*/, PyObject *args)
{
  LOG(INFO) << "sync_func";
  PyObject *pyengine, *pydepsgraph, *pycontext, *pysettings;

  if (!PyArg_ParseTuple(args, "OOOO", &pyengine, &pydepsgraph, &pycontext, &pysettings)) {
    Py_RETURN_NONE;
  }

  Engine *engine = (Engine *)PyLong_AsVoidPtr(pyengine);

  PointerRNA depsgraphptr;
  RNA_pointer_create(NULL, &RNA_Context, (ID *)PyLong_AsVoidPtr(pydepsgraph), &depsgraphptr);
  BL::Depsgraph b_depsgraph(depsgraphptr);

  PointerRNA contextptr;
  RNA_pointer_create(NULL, &RNA_Context, (ID *)PyLong_AsVoidPtr(pycontext), &contextptr);
  BL::Context b_context(contextptr);

  HdRenderSettingsMap settings;
  PyObject *pyiter = PyObject_GetIter(pysettings);
  if (pyiter) {
    PyObject *pykey, *pyval;
    while (pykey = PyIter_Next(pyiter)) {
      TfToken key(PyUnicode_AsUTF8(pykey));
      pyval = PyDict_GetItem(pysettings, pykey);
      if (PyLong_Check(pyval)) {
        settings[key] = PyLong_AsLong(pyval);
      }
      else if (PyFloat_Check(pyval)) {
        settings[key] = PyFloat_AsDouble(pyval);
      }
      else if (PyUnicode_Check(pyval)) {
        settings[key] = PyUnicode_AsUTF8(pyval);
      }
    }
  }

  engine->sync(b_depsgraph, b_context, settings);
  Py_RETURN_NONE;
}

static PyObject *render_func(PyObject * /*self*/, PyObject *args)
{
  LOG(INFO) << "render_func";
  PyObject *pyengine, *pydepsgraph;

  if (!PyArg_ParseTuple(args, "OO", &pyengine, &pydepsgraph)) {
    Py_RETURN_NONE;
  }

  FinalEngine *engine = (FinalEngine *)PyLong_AsVoidPtr(pyengine);

  PointerRNA depsgraphptr;
  RNA_pointer_create(NULL, &RNA_Depsgraph, (ID *)PyLong_AsVoidPtr(pydepsgraph), &depsgraphptr);
  BL::Depsgraph depsgraph(depsgraphptr);

  /* Allow Blender to execute other Python scripts. */
  Py_BEGIN_ALLOW_THREADS
    engine->render(depsgraph);
  Py_END_ALLOW_THREADS

  Py_RETURN_NONE;
}

static PyObject *view_draw_func(PyObject * /*self*/, PyObject *args)
{
  LOG(INFO) << "view_draw_func";

  PyObject *pyengine, *pydepsgraph, *pycontext;

  if (!PyArg_ParseTuple(args, "OOO", &pyengine, &pydepsgraph, &pycontext)) {
    Py_RETURN_NONE;
  }

  ViewportEngine *engine = (ViewportEngine *)PyLong_AsVoidPtr(pyengine);

  PointerRNA depsgraphptr;
  RNA_pointer_create(NULL, &RNA_Depsgraph, (ID *)PyLong_AsVoidPtr(pydepsgraph), &depsgraphptr);
  BL::Depsgraph b_depsgraph(depsgraphptr);

  PointerRNA contextptr;
  RNA_pointer_create(NULL, &RNA_Context, (ID *)PyLong_AsVoidPtr(pycontext), &contextptr);
  BL::Context b_context(contextptr);

  /* Allow Blender to execute other Python scripts. */
  Py_BEGIN_ALLOW_THREADS
    engine->viewDraw(b_depsgraph, b_context);
  Py_END_ALLOW_THREADS

  Py_RETURN_NONE;
}

static PyObject* get_render_plugins_func(PyObject* /*self*/, PyObject* args)
{
  PlugRegistry &registry = PlugRegistry::GetInstance();
  TfTokenVector pluginsIds = UsdImagingGLEngine::GetRendererPlugins();
  PyObject *ret = PyTuple_New(pluginsIds.size());
  for (int i = 0; i < pluginsIds.size(); ++i) {
    PyObject *descr = PyDict_New();
    PyDict_SetItemString(descr, "id", PyUnicode_FromString(pluginsIds[i].GetText()));
    PyDict_SetItemString(descr, "name", PyUnicode_FromString(UsdImagingGLEngine::GetRendererDisplayName(pluginsIds[i]).c_str()));

    std::string plugin_name = pluginsIds[i];
    plugin_name = plugin_name.substr(0, plugin_name.size()-6);
    plugin_name[0] = tolower(plugin_name[0]);
    std::string path = "";
    PlugPluginPtr plugin = registry.GetPluginWithName(plugin_name);
    if (plugin) {
        path = plugin->GetPath();
    }
    PyDict_SetItemString(descr, "path", PyUnicode_FromString(path.c_str()));

    PyTuple_SetItem(ret, i, descr);
  }
  return ret;
}

static PyMethodDef methods[] = {
  {"create", create_func, METH_VARARGS, ""},
  {"free", free_func, METH_VARARGS, ""},
  {"render", render_func, METH_VARARGS, ""},
  {"sync", sync_func, METH_VARARGS, ""},
  {"view_draw", view_draw_func, METH_VARARGS, ""},
  {"get_render_plugins", get_render_plugins_func, METH_VARARGS, ""},
  {NULL, NULL, 0, NULL},
};

static struct PyModuleDef module = {
  PyModuleDef_HEAD_INIT,
  "engine",
  "",
  -1,
  methods,
  NULL,
  NULL,
  NULL,
  NULL,
};

PyObject *addPythonSubmodule_engine(PyObject *mod)
{
  PyObject *submodule = PyModule_Create(&module);
  PyModule_AddObject(mod, "engine", submodule);
  return submodule;
}

}   // namespace usdhydra
