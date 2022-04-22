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

static void *compute_BlenderDataNode(void *node)
{
  std::cout << "BlenderDataNode" << std::endl;
  return NULL;
}

static void *compute_UsdFileNode(void *node)
{
  std::cout << "UsdFileNode" << std::endl;
  return NULL;
}

static void *compute_HydraRenderNode(void *node)
{
  std::cout << "HydraRenderNode" << std::endl;
  return NULL;
}

static void *compute_WriteFileNode(void *node)
{
  std::cout << "WriteFileNode" << std::endl;
  return NULL;
}

static void *compute_MergeNode(void *node)
{
  std::cout << "MergeNode" << std::endl;
  return NULL;
}

static void *compute_FilterNode(void *node)
{
  std::cout << "FilterNode" << std::endl;
  return NULL;
}

static void *compute_RootNode(void *node)
{
  std::cout << "RootNode" << std::endl;
  return NULL;
}

static void *compute_InstancingNode(void *node)
{
  std::cout << "InstancingNode" << std::endl;
  return NULL;
}

static void *compute_TransformNode(void *node)
{
  std::cout << "TransformNode" << std::endl;
  return NULL;
}

static void *compute_TransformByEmptyNode(void *node)
{
  std::cout << "TransformByEmptyNode" << std::endl;
  return NULL;
}

static PyObject *compute(PyObject *self, PyObject *args)
{
  USDNodeType usdNodeType;
  void *node = NULL;
  void *stage = NULL;

  if (!PyArg_ParseTuple(args, "iO:ref", &usdNodeType, &node)) {
    Py_RETURN_NONE;
  }

  if (!node || node == Py_None) {
    Py_RETURN_NONE;
  }

  switch (usdNodeType) {
    case USDNodeType::BlenderDataNode:
      stage = compute_BlenderDataNode(node);
      break;

    case USDNodeType::UsdFileNode:
      stage = compute_UsdFileNode(node);
      break;

    case USDNodeType::HydraRenderNode:
      stage = compute_HydraRenderNode(node);
      break;

    case USDNodeType::WriteFileNode:
      stage = compute_WriteFileNode(node);
      break;

    case USDNodeType::MergeNode:
      stage = compute_MergeNode(node);
      break;

    case USDNodeType::FilterNode:
      stage = compute_FilterNode(node);
      break;

    case USDNodeType::RootNode:
      stage = compute_RootNode(node);
      break;

    case USDNodeType::InstancingNode:
      stage = compute_InstancingNode(node);
      break;

    case USDNodeType::TransformNode:
      stage = compute_TransformNode(node);
      break;

    case USDNodeType::TransformByEmptyNode:
      stage = compute_TransformByEmptyNode(node);
      break;
  }

  return PyLong_FromVoidPtr(stage);
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
