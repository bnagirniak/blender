/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <pxr/base/plug/plugin.h>
#include <pxr/base/plug/registry.h>
#include <pxr/usd/usdGeom/tokens.h>

#include "glog/logging.h"

#include "intern/usd_hierarchy_iterator.h"
#include "BKE_context.h"
#include "BKE_blender_version.h"
#include "DEG_depsgraph_query.h"

#include "engine.h"

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

/* ------------------------------------------------------------------------- */
/* Python API for Engine
 */

static PyObject *create_func(PyObject * /*self*/, PyObject *args)
{
  LOG(INFO) << "create_func";

  PyObject *b_pyengine;
  char *engineType, *delegateId;
  if (!PyArg_ParseTuple(args, "Oss", &b_pyengine, &engineType, &delegateId)) {
    Py_RETURN_NONE;
  }

  PointerRNA b_engineptr;
  RNA_pointer_create(NULL, &RNA_RenderEngine, (void *)PyLong_AsVoidPtr(b_pyengine), &b_engineptr);
  BL::RenderEngine b_engine(b_engineptr);

  Engine *engine;
  if (std::string(engineType) == "VIEWPORT") {
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
      Py_DECREF(pykey);
    }
    Py_DECREF(pyiter);
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
  LOG(INFO) << "get_render_plugins_func";

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

static PyObject *stage_export_to_str_func(PyObject * /*self*/, PyObject *args)
{
  LOG(INFO) << "stage_export_to_str_func";

  PyObject *pyengine;
  int flatten;
  if (!PyArg_ParseTuple(args, "Op", &pyengine, &flatten)) {
    Py_RETURN_NONE;
  }

  Engine *engine = (Engine *)PyLong_AsVoidPtr(pyengine);
  UsdStageRefPtr stage = engine->getStage();

  if (!stage) {
    Py_RETURN_NONE;
  }

  std::string str;
  if (flatten) {
    stage->ExportToString(&str);
  }
  else {
    stage->GetRootLayer()->ExportToString(&str);
  }
  return PyUnicode_FromString(str.c_str());
}

static PyMethodDef methods[] = {
  {"create", create_func, METH_VARARGS, ""},
  {"free", free_func, METH_VARARGS, ""},
  {"render", render_func, METH_VARARGS, ""},
  {"sync", sync_func, METH_VARARGS, ""},
  {"view_draw", view_draw_func, METH_VARARGS, ""},
  {"get_render_plugins", get_render_plugins_func, METH_VARARGS, ""},
  {"stage_export_to_str", stage_export_to_str_func, METH_VARARGS, ""},
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
