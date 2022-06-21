/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <pxr/usd/usd/prim.h>

#include "stage.h"

using namespace pxr;

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

  UsdStageRefPtr stage = stageCache->Find(UsdStageCache::Id::FromLongInt(stageId));

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

  UsdStageRefPtr stage = stageCache->Find(UsdStageCache::Id::FromLongInt(stageId));

  if (!stage) {
    Py_RETURN_FALSE;
  }

  stageCache->Erase(stage);

  Py_RETURN_TRUE;
}

static PyObject *prim_get_children_func(PyObject * /*self*/, PyObject *args)
{
  long stageId;
  char *path;
  if (!PyArg_ParseTuple(args, "ls", &stageId, &path)) {
    Py_RETURN_NONE;
  }

  UsdStageRefPtr stage = stageCache->Find(UsdStageCache::Id::FromLongInt(stageId));
  UsdPrim prim = stage->GetPrimAtPath(SdfPath(path));

  std::vector<std::string> childNames;
  for (UsdPrim child : prim.GetAllChildren()) {
    childNames.push_back(child.GetPath().GetAsString());
  }

  PyObject *ret = PyTuple_New(childNames.size());
  for (int i = 0; i < childNames.size(); ++i) {
    PyTuple_SetItem(ret, i, PyUnicode_FromString(childNames[i].c_str()));
  }

  return ret;
}

static PyMethodDef methods[] = {
  {"export_to_str", export_to_str_func, METH_VARARGS, ""},
  {"free", free_func, METH_VARARGS, ""},
  {"prim_get_children", prim_get_children_func, METH_VARARGS, ""},
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

