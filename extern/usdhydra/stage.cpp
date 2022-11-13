/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/imageable.h>

#include "stage.h"

namespace usdhydra {

unique_ptr<UsdStageCache> stageCache;

void stage_init()
{
  if (!stageCache) {
    stageCache = make_unique<UsdStageCache>();
  }
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

  string str;
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

static PyObject *prim_get_info_func(PyObject * /*self*/, PyObject *args)
{
  long stageId;
  char *path;
  if (!PyArg_ParseTuple(args, "ls", &stageId, &path)) {
    Py_RETURN_NONE;
  }

  UsdStageRefPtr stage = stageCache->Find(UsdStageCache::Id::FromLongInt(stageId));
  UsdPrim prim = stage->GetPrimAtPath(SdfPath(path));

  if (!prim) {
    Py_RETURN_NONE;
  }

  PyObject *ret = PyDict_New();
  PyDict_SetItemString(ret, "name", PyUnicode_FromString(prim.GetName().GetText()));
  PyDict_SetItemString(ret, "path", PyUnicode_FromString(prim.GetPath().GetText()));
  PyDict_SetItemString(ret, "type", PyUnicode_FromString(prim.GetTypeName().GetText()));

  bool visible = UsdGeomImageable(prim).ComputeVisibility().GetString() != "invisible";
  PyDict_SetItemString(ret, "visible", visible? Py_True: Py_False);

  auto childrenNames = prim.GetAllChildrenNames();
  PyObject *children = PyTuple_New(childrenNames.size());
  for (int i = 0; i < childrenNames.size(); ++i) {
    PyTuple_SetItem(children, i, PyUnicode_FromString(childrenNames[i].GetText()));
  }
  PyDict_SetItemString(ret, "children", children);

  return ret;
}

static PyObject *stage_get_info_func(PyObject * /*self*/, PyObject *args)
{
  long stageId;
  if (!PyArg_ParseTuple(args, "l", &stageId)) {
    Py_RETURN_NONE;
  }

  UsdStageRefPtr stage = stageCache->Find(UsdStageCache::Id::FromLongInt(stageId));

  PyObject *ret = PyDict_New();
  PyDict_SetItemString(ret, "filepath", PyUnicode_FromString(stage->GetRootLayer()->GetResolvedPath().GetPathString().c_str()));

  return ret;
}

static PyObject* traverse_stage_func(PyObject* /*self*/, PyObject* args)
{
  long stageId;
  if (!PyArg_ParseTuple(args, "l", &stageId)) {
    Py_RETURN_NONE;
  }

  vector<UsdPrim> usd_prims;

  UsdStageRefPtr stage = stageCache->Find(UsdStageCache::Id::FromLongInt(stageId));
  for (UsdPrim usd_prim : stage->TraverseAll()) {
    usd_prims.push_back(usd_prim);
  }

  PyObject *pyTuple = PyTuple_New(usd_prims.size());
  for (int i = 0; i < usd_prims.size(); ++i) {
    PyObject *pyDict = PyDict_New();
    PyDict_SetItemString(pyDict, "name", PyUnicode_FromString(usd_prims[i].GetName().GetText()));
    PyDict_SetItemString(pyDict, "path", PyUnicode_FromString(usd_prims[i].GetPath().GetText()));
    PyDict_SetItemString(pyDict, "type", PyUnicode_FromString(usd_prims[i].GetTypeName().GetText()));

    PyTuple_SetItem(pyTuple, i, pyDict);
  }

  return pyTuple;
}

static PyObject *get_xform_transform_func(PyObject * /*self*/, PyObject *args)
{
  long stageId;
  const char *path;
  if (!PyArg_ParseTuple(args, "ls", &stageId, &path)) {
    Py_RETURN_NONE;
  }

  UsdStageRefPtr stage = stageCache->Find(UsdStageCache::Id::FromLongInt(stageId));
  UsdPrim usd_prim = stage->GetPrimAtPath(SdfPath(path));
  UsdGeomXform xForm = UsdGeomXform(usd_prim);

  GfMatrix4d transform;
  bool resets_xform_stack;

  xForm.GetLocalTransformation(&transform, &resets_xform_stack);

  PyObject *pyMatrix = PyTuple_New(16);

  for (int i = 0; i < 16; ++i) {
    PyTuple_SetItem(pyMatrix, i, PyFloat_FromDouble(transform.GetArray()[i]));
  }

  return pyMatrix;
}

static PyMethodDef methods[] = {
  {"export_to_str", export_to_str_func, METH_VARARGS, ""},
  {"free", free_func, METH_VARARGS, ""},
  {"prim_get_info", prim_get_info_func, METH_VARARGS, ""},
  {"stage_get_info", stage_get_info_func, METH_VARARGS, ""},
  {"traverse_stage", traverse_stage_func, METH_VARARGS, ""},
  {"get_xform_transform", get_xform_transform_func, METH_VARARGS, ""},
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

}   // namespace usdhydra

