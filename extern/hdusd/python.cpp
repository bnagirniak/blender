/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#define GLOG_NO_ABBREVIATED_SEVERITIES

#include <iostream>

#include <Python.h>

#include <pxr/pxr.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/imaging/hd/tokens.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>
#include <pxr/base/gf/camera.h>
#include <pxr/base/plug/registry.h>

#include "glog/logging.h"
#include "usd_common.h"

#include "hdusd_python_api.h"
#include "session.h"
#include "utils.h"
#include "view_settings.h"

namespace hdusd {

static PyObject *init_func(PyObject * /*self*/, PyObject *args)
{
  DLOG(INFO) << "init_func";
  blender::io::usd::ensure_usd_plugin_path_registered();
  stageCache = std::make_unique<pxr::UsdStageCache>();

  Py_RETURN_NONE;
}

static PyObject *exit_func(PyObject * /*self*/, PyObject * /*args*/)
{
  DLOG(INFO) << "exit_func";
  stageCache = nullptr;
  Py_RETURN_NONE;
}

static PyObject *create_func(PyObject * /*self*/, PyObject *args)
{
  DLOG(INFO) << "create_func";
  PyObject *pyengine;
  if (!PyArg_ParseTuple(args, "O", &pyengine)) {
    Py_RETURN_NONE;
  }

  PointerRNA engineptr;
  RNA_pointer_create(NULL, &RNA_RenderEngine, (void *)PyLong_AsVoidPtr(pyengine), &engineptr);
  BL::RenderEngine engine(engineptr);

  /* create session */
  BlenderSession *session = new BlenderSession(engine);
  return PyLong_FromVoidPtr(session);
}

static PyObject *free_func(PyObject * /*self*/, PyObject *args)
{
  DLOG(INFO) << "free_func";
  PyObject *pysession;
  if (!PyArg_ParseTuple(args, "O", &pysession)) {
    Py_RETURN_NONE;
  }

  delete (BlenderSession *)PyLong_AsVoidPtr(pysession);
  Py_RETURN_NONE;
}

static PyObject *render_func(PyObject * /*self*/, PyObject *args)
{
  DLOG(INFO) << "render_func";
  PyObject *pysession, *pydepsgraph;

  if (!PyArg_ParseTuple(args, "OO", &pysession, &pydepsgraph)) {
    Py_RETURN_NONE;
  }

  BlenderSession *session = (BlenderSession *)PyLong_AsVoidPtr(pysession);

  PointerRNA depsgraphptr;
  RNA_pointer_create(NULL, &RNA_Depsgraph, (ID *)PyLong_AsVoidPtr(pydepsgraph), &depsgraphptr);
  BL::Depsgraph depsgraph(depsgraphptr);

  ///* Allow Blender to execute other Python scripts. */
  //python_thread_state_save(&session->python_thread_state);

  //session->render(b_depsgraph);

  //python_thread_state_restore(&session->python_thread_state);

  Py_RETURN_NONE;
}

static PyObject *reset_func(PyObject * /*self*/, PyObject *args)
{
  DLOG(INFO) << "reset_func";
  PyObject *pysession, *pydata, *pydepsgraph;
  int stageId = 0;
  if (!PyArg_ParseTuple(args, "OOOi", &pysession, &pydata, &pydepsgraph, &stageId)) {
    Py_RETURN_NONE;
  }

  BlenderSession *session = (BlenderSession *)PyLong_AsVoidPtr(pysession);

  PointerRNA dataptr;
  RNA_main_pointer_create((Main *)PyLong_AsVoidPtr(pydata), &dataptr);
  BL::BlendData data(dataptr);

  PointerRNA depsgraphptr;
  RNA_pointer_create(NULL, &RNA_Depsgraph, (ID *)PyLong_AsVoidPtr(pydepsgraph), &depsgraphptr);
  BL::Depsgraph depsgraph(depsgraphptr);

  pxr::TfToken plugin = pxr::TfToken("HdStormRendererPlugin");
  if (!session->imagingGLEngine->SetRendererPlugin(plugin)) {
    Py_RETURN_NONE;
  }

  session->stage = stageCache->Find(pxr::UsdStageCache::Id::FromLongInt(stageId));
  session->imagingGLEngine->SetRendererAov(pxr::HdAovTokens->color);
  Py_RETURN_NONE;
}

static PyObject *render_frame_finish_func(PyObject * /*self*/, PyObject *args)
{
  Py_RETURN_NONE;
}

static PyObject *view_update_func(PyObject * /*self*/, PyObject *args)
{
  DLOG(INFO) << "view_update_func";
  Py_RETURN_NONE;
}

static PyObject *view_draw_func(PyObject * /*self*/, PyObject *args)
{
  DLOG(INFO) << "view_draw_func";

  PyObject *pysession, *pydepsgraph, *pycontext, *pyspaceData, *pyregionData;

  if (!PyArg_ParseTuple(args, "OOOOO", &pysession, &pydepsgraph, &pycontext, &pyspaceData, &pyregionData)) {
    Py_RETURN_NONE;
  }

  BlenderSession *session = (BlenderSession *)PyLong_AsVoidPtr(pysession);

  PointerRNA depsgraphptr;
  RNA_pointer_create(NULL, &RNA_Depsgraph, (ID *)PyLong_AsVoidPtr(pydepsgraph), &depsgraphptr);
  BL::Depsgraph depsgraph(depsgraphptr);

  PointerRNA contextptr;
  RNA_pointer_create(NULL, &RNA_Context, (ID *)PyLong_AsVoidPtr(pycontext), &contextptr);
  BL::Context b_context(contextptr);

  BL::Scene b_scene = depsgraph.scene_eval();
  BL::RenderEngine b_engine = session->b_engine;
  
  ViewSettings *view_settings = new ViewSettings(b_context);

  if (view_settings->get_width() * view_settings->get_height() == 0) {
    Py_RETURN_NONE;
  };

  pxr::GfCamera gf_camera = view_settings->export_camera();

  vector<pxr::GfVec4f> clip_planes = gf_camera.GetClippingPlanes();

  for (int i = 0; i < clip_planes.size(); i++) {
    session->render_params.clipPlanes.push_back((pxr::GfVec4d)clip_planes[i]);
  }

  session->imagingGLEngine->SetCameraState(gf_camera.GetFrustum().ComputeViewMatrix(),
                                  gf_camera.GetFrustum().ComputeProjectionMatrix());
  session->imagingGLEngine->SetRenderViewport(pxr::GfVec4d((double)view_settings->border[0][0], (double)view_settings->border[0][1],
                                                  (double)view_settings->border[1][0], (double)view_settings->border[1][1]));

  b_engine.bind_display_space_shader(b_scene);
  
  session->imagingGLEngine->Render(session->stage->GetPseudoRoot(), session->render_params);

  b_engine.unbind_display_space_shader();

  
  ///* Allow Blender to execute other Python scripts. */
  //python_thread_state_save(&session->python_thread_state);

  //session->render(b_depsgraph);

  //python_thread_state_restore(&session->python_thread_state);

  Py_RETURN_NONE;
}

static PyObject *stage_export_to_str_func(PyObject * /*self*/, PyObject *args)
{
  long stageId;
  int flutten;

  if (!PyArg_ParseTuple(args, "lp", &stageId, &flutten)) {
    Py_RETURN_NONE;
  }

  pxr::UsdStageRefPtr stage = stageCache->Find(pxr::UsdStageCache::Id::FromLongInt(stageId));

  if (!stage) {
    Py_RETURN_NONE;
  }

  std::string str;
  if (flutten) {
    stage->ExportToString(&str);
  }
  else {
    stage->GetRootLayer()->ExportToString(&str);
  }
  return PyUnicode_FromString(str.c_str());
}

static PyObject *stage_free_func(PyObject * /*self*/, PyObject *args)
{
  long stageId;

  if (!PyArg_ParseTuple(args, "l", &stageId)) {
    Py_RETURN_FALSE;
  }

  pxr::UsdStageRefPtr stage = stageCache->Find(pxr::UsdStageCache::Id::FromLongInt(stageId));

  if (!stage) {
    Py_RETURN_FALSE;
  }

  stageCache->Erase(stage);
  DLOG(INFO) << "stage_free "<< stageId;

  Py_RETURN_TRUE;
}

static PyObject *get_temp_dir_func(PyObject * /*self*/, PyObject * /*args*/)
{
  DLOG(INFO) << "get_temp_dir_func";
  filesystem::path path = hdusd::get_temp_dir();
  std::string str{path.u8string()};
  return PyUnicode_FromString(str.c_str());
}

static PyObject *test_func(PyObject * /*self*/, PyObject *args)
{
  char *path;
  if (!PyArg_ParseTuple(args, "s", &path)) {
    Py_RETURN_NONE;
  }

  printf("%s\n", path);

  pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(path);

  if (!stage) {
    Py_RETURN_NONE;
  }
  std::string str;
  stage->ExportToString(&str);
  printf("%s\n", str.c_str());

  Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
  {"init", init_func, METH_VARARGS, ""},
  {"exit", exit_func, METH_VARARGS, ""},
  {"create", create_func, METH_VARARGS, ""},
  {"free", free_func, METH_VARARGS, ""},
  {"render", render_func, METH_VARARGS, ""},
  {"reset", reset_func, METH_VARARGS, ""},
  {"render_frame_finish", render_frame_finish_func, METH_VARARGS, ""},
  {"view_update", view_update_func, METH_VARARGS, ""},
  {"view_draw", view_draw_func, METH_VARARGS, ""},

  {"stage_export_to_str", stage_export_to_str_func, METH_VARARGS, ""},
  {"stage_free", stage_free_func, METH_VARARGS, ""},
  {"get_temp_dir", get_temp_dir_func, METH_VARARGS, ""},

  {"test", test_func, METH_VARARGS, ""},
  {NULL, NULL, 0, NULL},
};

static struct PyModuleDef module = {
  PyModuleDef_HEAD_INIT,
  "_hdusd",
  "HdUSD render integration",
  -1,
  methods,
  NULL,
  NULL,
  NULL,
  NULL,
};

}   // namespace hdusd

PyObject *HdUSD_initPython(void)
{
  PyObject *mod = PyModule_Create(&hdusd::module);
  PyObject *submodule = HdUSD_usd_node_initPython();

  PyModule_AddObject(mod, "usd_node", submodule);

  return mod;
}
