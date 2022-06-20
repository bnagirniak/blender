/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <iostream>

#include <Python.h>

#include "usd_common.h"
#include "glog/logging.h"

#include "stage.h"
#include "usd_node.h"
#include "session.h"
#include "utils.h"


namespace hdusd {

static PyObject *init_func(PyObject * /*self*/, PyObject *args)
{
  LOG(INFO) << "init_func";
  blender::io::usd::ensure_usd_plugin_path_registered();
  stage_init();

  Py_RETURN_NONE;
}

static PyObject *exit_func(PyObject * /*self*/, PyObject * /*args*/)
{
  LOG(INFO) << "exit_func";
  stageCache = nullptr;
  Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
  {"init", init_func, METH_VARARGS, ""},
  {"exit", exit_func, METH_VARARGS, ""},
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

#ifdef __cplusplus
extern "C" {
#endif

PyObject *HdUSD_initPython(void)
{
  PyObject *mod = PyModule_Create(&hdusd::module);
  hdusd::addPythonSubmodule_stage(mod);
  hdusd::addPythonSubmodule_usd_node(mod);
  hdusd::addPythonSubmodule_session(mod);
  hdusd::addPythonSubmodule_utils(mod);

  return mod;
}

#ifdef __cplusplus
}
#endif
