/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include "stage.h"

namespace hdusd {

std::unique_ptr<pxr::UsdStageCache> stageCache;

void stage_init()
{
  stageCache = std::make_unique<pxr::UsdStageCache>();
}

static PyObject *export_to_str_func(PyObject * /*self*/, PyObject *args)
{
  long stageId;
  int flatten;

  if (!PyArg_ParseTuple(args, "lp", &stageId, &flatten)) {
    Py_RETURN_NONE;
  }

  pxr::UsdStageRefPtr stage = stageCache->Find(pxr::UsdStageCache::Id::FromLongInt(stageId));

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

static PyObject *free_func(PyObject * /*self*/, PyObject *args)
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

  Py_RETURN_TRUE;
}

static PyMethodDef methods[] = {
  {"export_to_str", export_to_str_func, METH_VARARGS, ""},
  {"free", free_func, METH_VARARGS, ""},
  {NULL, NULL, 0, NULL},
};

static struct PyModuleDef module = {
  PyModuleDef_HEAD_INIT,
  "stage",
  "This module provides access to USD Stage related functions.",
  -1,
  methods,
  NULL,
  NULL,
  NULL,
  NULL,
};

PyObject *addPythonSubmodule_stage(PyObject *mod)
{
  PyObject *submodule = PyModule_Create(&module);
  PyModule_AddObject(mod, "stage", submodule);
  return submodule;
}

}   // namespace hdusd

