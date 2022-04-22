/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <Python.h>
#include <iostream>

#include "hdusd_python_api.h"

enum class USDNodeType
{
  BlenderDataNode,
  UsdFileNode,
  HydraRenderNode,
  WriteFileNode,
  MergeNode,
  FilterNode,
  RootNode,
  InstancingNode,
  TransformNode,
  TransformByEmptyNode
};

static void *_compute_BlenderDataNode(void *stage)
{
  std::cout << "BlenderDataNode" << std::endl;
  return NULL;
}

static void *_compute_UsdFileNode(void *stage)
{
  std::cout << "UsdFileNode" << std::endl;
  return NULL;
}

static void *_compute_HydraRenderNode(void *stage)
{
  std::cout << "HydraRenderNode" << std::endl;
  return NULL;
}

static void *_compute_WriteFileNode(void *stage)
{
  std::cout << "WriteFileNode" << std::endl;
  return NULL;
}

static void *_compute_MergeNode(void *stage)
{
  std::cout << "MergeNode" << std::endl;
  return NULL;
}

static void *_compute_FilterNode(void *stage)
{
  std::cout << "FilterNode" << std::endl;
  return NULL;
}

static void *_compute_RootNode(void *stage)
{
  std::cout << "RootNode" << std::endl;
  return NULL;
}

static void *_compute_InstancingNode(void *stage)
{
  std::cout << "InstancingNode" << std::endl;
  return NULL;
}

static void *_compute_TransformNode(void *stage)
{
  std::cout << "TransformNode" << std::endl;
  return NULL;
}

static void *_compute_TransformByEmptyNode(void *stage)
{
  std::cout << "TransformByEmptyNode" << std::endl;
  return NULL;
}

static PyObject *compute(PyObject *self, PyObject *args)
{
  USDNodeType usdNodeType;
  void *stage = NULL;

  if (!PyArg_ParseTuple(args, "iO:ref", &usdNodeType, &stage)) Py_RETURN_NONE;

  if (!stage || stage == Py_None) Py_RETURN_NONE;

  switch (usdNodeType)
  {
    case USDNodeType::BlenderDataNode:
      _compute_BlenderDataNode(stage);
      break;

    case USDNodeType::UsdFileNode:
      _compute_UsdFileNode(stage);
      break;

    case USDNodeType::HydraRenderNode:
      _compute_HydraRenderNode(stage);
      break;

    case USDNodeType::WriteFileNode:
      _compute_WriteFileNode(stage);
      break;

    case USDNodeType::MergeNode:
      _compute_MergeNode(stage);
      break;

    case USDNodeType::FilterNode:
      _compute_FilterNode(stage);
      break;

    case USDNodeType::RootNode:
      _compute_RootNode(stage);
      break;

    case USDNodeType::InstancingNode:
      _compute_InstancingNode(stage);
      break;

    case USDNodeType::TransformNode:
      _compute_TransformNode(stage);
      break;

    case USDNodeType::TransformByEmptyNode:
      _compute_TransformByEmptyNode(stage);
      break;
  }

  return Py_BuildValue("O", stage);
}

static PyMethodDef methods[] = {
  {"compute", compute, METH_VARARGS, ""},
  {NULL, NULL, 0, NULL},
};

static struct PyModuleDef module = {
  PyModuleDef_HEAD_INIT,
  "usd_node",
  "This module provides access to USD nodes evaluation functions.",
  -1,
  methods,
  NULL,
  NULL,
  NULL,
  NULL,
};

static struct PyModuleDef usdNodeTypeModule = {
  PyModuleDef_HEAD_INIT,
  "usdNodeType",
  "This module defines USD node types.",
  -1,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};

PyObject *HdUSD_usd_node_initPython(void)
{
  PyObject *mod = PyModule_Create(&module);
  PyObject *usdNodeTypeMod = PyModule_Create(&usdNodeTypeModule);

  PyModule_AddObject(usdNodeTypeMod, "BlenderDataNode", PyLong_FromLong((int)USDNodeType::BlenderDataNode));
  PyModule_AddObject(usdNodeTypeMod, "UsdFileNode", PyLong_FromLong((int)USDNodeType::UsdFileNode));
  PyModule_AddObject(usdNodeTypeMod, "HydraRenderNode", PyLong_FromLong((int)USDNodeType::HydraRenderNode));
  PyModule_AddObject(usdNodeTypeMod, "WriteFileNode", PyLong_FromLong((int)USDNodeType::WriteFileNode));
  PyModule_AddObject(usdNodeTypeMod, "MergeNode", PyLong_FromLong((int)USDNodeType::MergeNode));
  PyModule_AddObject(usdNodeTypeMod, "FilterNode", PyLong_FromLong((int)USDNodeType::FilterNode));
  PyModule_AddObject(usdNodeTypeMod, "RootNode", PyLong_FromLong((int)USDNodeType::RootNode));
  PyModule_AddObject(usdNodeTypeMod, "InstancingNode", PyLong_FromLong((int)USDNodeType::InstancingNode));
  PyModule_AddObject(usdNodeTypeMod, "TransformNode", PyLong_FromLong((int)USDNodeType::TransformNode));
  PyModule_AddObject(usdNodeTypeMod, "TransformByEmptyNode", PyLong_FromLong((int)USDNodeType::TransformByEmptyNode));

  PyModule_AddObject(mod, "type", usdNodeTypeMod);

  return mod;
}
