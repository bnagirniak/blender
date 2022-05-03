/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <Python.h>

#include <pxr/pxr.h>
#include <pxr/usd/usd/stage.h>

#include "usd_common.h"

#include "hdusd_python_api.h"
#include "session.h"

static PyObject *init_func(PyObject * /*self*/, PyObject *args)
{
  blender::io::usd::ensure_usd_plugin_path_registered();
  Py_RETURN_NONE;
}

static PyObject *exit_func(PyObject * /*self*/, PyObject * /*args*/)
{
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

  return PyLong_FromVoidPtr(session);
}

static PyObject *free_func(PyObject * /*self*/, PyObject *value)
{
  delete (BlenderSession *)PyLong_AsVoidPtr(value);

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

static PyObject *view_draw_func(PyObject * /*self*/, PyObject *args)
{
  Py_RETURN_NONE;
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
  {"view_draw", view_draw_func, METH_VARARGS, ""},
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

PyObject *HdUSD_initPython(void)
{
  PyObject *mod = PyModule_Create(&module);
  PyObject *submodule = HdUSD_usd_node_initPython();

  PyModule_AddObject(mod, "usd_node", submodule);

  return mod;
}
