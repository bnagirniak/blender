/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */


#include <iostream>

#include <Python.h>

#include "usd_common.h"

#include "hdusd_python_api.h"

#include "usd.h"
#include "usd_node.h"
#include "session.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include "glog/logging.h"

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
  hdusd::usd_addPythonSubmodule(mod);
  hdusd::usd_node_addPythonSubmodule(mod);
  hdusd::session_addPythonSubmodule(mod);

  return mod;
}
