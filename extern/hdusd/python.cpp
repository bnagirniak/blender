/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

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

#include <GL/glew.h>

#include "usd_common.h"

#include "hdusd_python_api.h"
#include "session.h"
#include "view_settings.h"

pxr::UsdImagingGLRenderParams render_params;
pxr::UsdStageRefPtr stage;
pxr::GfCamera gf_camera;

namespace hdusd {

static PyObject *init_func(PyObject * /*self*/, PyObject *args)
{
  blender::io::usd::ensure_usd_plugin_path_registered();
  stageCache = std::make_unique<pxr::UsdStageCache>();

  Py_RETURN_NONE;
}

static PyObject *exit_func(PyObject * /*self*/, PyObject * /*args*/)
{
  stageCache = nullptr;
  imagingGLEngine.reset();
  stage.Reset();
  Py_RETURN_NONE;
}

static PyObject *create_func(PyObject * /*self*/, PyObject *args)
{
  PyObject *pyengine, *pydata;
  if (!PyArg_ParseTuple(args, "OO", &pyengine, &pydata)) {
    return NULL;
  }

  PointerRNA engineptr;
  RNA_pointer_create(NULL, &RNA_RenderEngine, (void *)PyLong_AsVoidPtr(pyengine), &engineptr);
  BL::RenderEngine engine(engineptr);

  PointerRNA dataptr;
  RNA_main_pointer_create((Main *)PyLong_AsVoidPtr(pydata), &dataptr);
  BL::BlendData data(dataptr);

  /* create session */
  BlenderSession *session = new BlenderSession(engine, data);

  //UsdImagingLiteEngine renderer = UsdImagingLiteEngine();
  //TfToken plugin = TfToken("HdRprPlugin");

  if (imagingGLEngine == nullptr) {
    imagingGLEngine = std::make_unique<pxr::UsdImagingGLEngine>();
  }

  pxr::TfToken plugin = pxr::TfToken("HdStormRendererPlugin");

  if (!imagingGLEngine->SetRendererPlugin(plugin)) {
    Py_RETURN_NONE;
  }

  render_params = pxr::UsdImagingGLRenderParams();
  vector<pxr::GfVec4f> clipPlanes = gf_camera.GetClippingPlanes();

  for (int i = 0; i < clipPlanes.size(); i++) {
    render_params.clipPlanes.push_back((pxr::GfVec4d)clipPlanes[i]);
  }

  stage = pxr::UsdStage::Open("C:/Users/user/Downloads/untitled_2.usda");

  imagingGLEngine->SetRendererAov(pxr::HdAovTokens->color);

  return PyLong_FromVoidPtr(session);
}

static PyObject *free_func(PyObject * /*self*/, PyObject *value)
{
  delete (BlenderSession *)PyLong_AsVoidPtr(value);

  imagingGLEngine.reset();
  stage.Reset();

  Py_RETURN_NONE;
}

static PyObject *render_func(PyObject * /*self*/, PyObject *args)
{
  PyObject *pysession, *pydepsgraph;

  if (!PyArg_ParseTuple(args, "OO", &pysession, &pydepsgraph))
    return NULL;

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
  PyObject *pysession, *pydata, *pydepsgraph;

  if (!PyArg_ParseTuple(args, "OOO", &pysession, &pydata, &pydepsgraph))
    return NULL;

  BlenderSession *session = (BlenderSession *)PyLong_AsVoidPtr(pysession);

  PointerRNA dataptr;
  RNA_main_pointer_create((Main *)PyLong_AsVoidPtr(pydata), &dataptr);
  BL::BlendData data(dataptr);

  PointerRNA depsgraphptr;
  RNA_pointer_create(NULL, &RNA_Depsgraph, (ID *)PyLong_AsVoidPtr(pydepsgraph), &depsgraphptr);
  BL::Depsgraph depsgraph(depsgraphptr);

  ///* Allow Blender to execute other Python scripts. */
  //python_thread_state_save(&session->python_thread_state);

  //session->render(b_depsgraph);

  //python_thread_state_restore(&session->python_thread_state);

  Py_RETURN_NONE;
}

static PyObject *render_frame_finish_func(PyObject * /*self*/, PyObject *args)
{
  Py_RETURN_NONE;
}

static PyObject *view_update_func(PyObject * /*self*/, PyObject *args)
{
  Py_RETURN_NONE;
}

static PyObject *view_draw_func(PyObject * /*self*/, PyObject *args)
{
  PyObject *pysession, *pydepsgraph, *pycontext, *pyspaceData, *pyregionData;

  if (!PyArg_ParseTuple(args, "OOOOO", &pysession, &pydepsgraph, &pycontext, &pyspaceData, &pyregionData))
    return NULL;

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

  gf_camera = view_settings->export_camera();

  imagingGLEngine->SetCameraState(gf_camera.GetFrustum().ComputeViewMatrix(),
                                  gf_camera.GetFrustum().ComputeProjectionMatrix());
  imagingGLEngine->SetRenderViewport(pxr::GfVec4d((double)view_settings->border[0][0], (double)view_settings->border[0][1],
                                                  (double)view_settings->border[1][0], (double)view_settings->border[1][1]));

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  b_engine.bind_display_space_shader(b_scene);

  imagingGLEngine->Render(stage->GetPseudoRoot(), render_params);

  b_engine.unbind_display_space_shader();

  glClear(GL_DEPTH_BUFFER_BIT);

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
  std::cout << "stage_free " << stageId << std::endl;

  Py_RETURN_TRUE;
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
