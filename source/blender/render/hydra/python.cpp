/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <iostream>
#include <cstdlib>

#include <Python.h>

#include <pxr/pxr.h>
#include <pxr/base/plug/plugin.h>
#include <pxr/base/plug/registry.h>

#include "glog/logging.h"
#include "BKE_appdir.h"

#include "engine.h"

using namespace std;

namespace blender::render::hydra {

static PyObject *init_func(PyObject * /*self*/, PyObject *args)
{
  LOG(INFO) << "init_func";

  pxr::PlugRegistry::GetInstance().RegisterPlugins(std::string(BKE_appdir_program_dir()) + "/blender.shared/usd");

  Py_RETURN_NONE;
}

static PyObject *register_plugins_func(PyObject * /*self*/, PyObject *args)
{
  PyObject *pyplugin_dirs, *pypath_dirs;
  if (!PyArg_ParseTuple(args, "OO", &pyplugin_dirs, &pypath_dirs)) {
    Py_RETURN_NONE;
  }

  LOG(INFO) << "register_plugins_func";

  vector<string> plugin_dirs, path_dirs;
  PyObject *pyiter, *pyitem;

  pyiter = PyObject_GetIter(pyplugin_dirs);
  if (pyiter) {
    while (pyitem = PyIter_Next(pyiter)) {
      plugin_dirs.push_back(PyUnicode_AsUTF8(pyitem));
      Py_DECREF(pyitem);
    }
    Py_DECREF(pyiter);
  }

  pyiter = PyObject_GetIter(pypath_dirs);
  if (pyiter) {
    while (pyitem = PyIter_Next(pyiter)) {
      path_dirs.push_back(PyUnicode_AsUTF8(pyitem));
      Py_DECREF(pyitem);
    }
    Py_DECREF(pyiter);
  }
  
  if (!path_dirs.empty()) {
    stringstream ss;
    ss << "PATH=";
    for (string &s : path_dirs) {
      ss << s;
#ifdef _WIN32
      ss << ";";
#else
      ss << ":";
#endif
    }
    ss << getenv("PATH");
    putenv(ss.str().c_str());
  }

  pxr::PlugRegistry &registry = pxr::PlugRegistry::GetInstance();
  registry.RegisterPlugins(plugin_dirs);

  Py_RETURN_NONE;
}

static PyObject *exit_func(PyObject * /*self*/, PyObject * /*args*/)
{
  LOG(INFO) << "exit_func";
  Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
  {"init", init_func, METH_VARARGS, ""},
  {"register_plugins", register_plugins_func, METH_VARARGS, ""},
  {"exit", exit_func, METH_VARARGS, ""},
  {NULL, NULL, 0, NULL},
};

static struct PyModuleDef module = {
  PyModuleDef_HEAD_INIT,
  "_hydra",
  "Hydra render API",
  -1,
  methods,
  NULL,
  NULL,
  NULL,
  NULL,
};

} // namespace blender::render::hydra

#ifdef __cplusplus
extern "C" {
#endif

PyObject *Hydra_initPython(void)
{
  PyObject *mod = PyModule_Create(&blender::render::hydra::module);
  blender::render::hydra::addPythonSubmodule_engine(mod);

  return mod;
}

#ifdef __cplusplus
}
#endif
