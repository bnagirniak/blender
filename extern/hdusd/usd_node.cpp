/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <Python.h>
#include <iostream>

#include <pxr/usd/usd/stage.h>

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

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

static pxr::UsdStageRefPtr compute_BlenderDataNode(PyObject *nodeArgs)
{
  std::cout << "BlenderDataNode" << std::endl;
  return nullptr;
}

static pxr::UsdStageRefPtr compute_UsdFileNode(PyObject *nodeArgs)
{
  std::cout << "UsdFileNode" << std::endl;
  return NULL;
}

static pxr::UsdStageRefPtr compute_HydraRenderNode(PyObject *nodeArgs)
{
  std::cout << "HydraRenderNode" << std::endl;
  return NULL;
}

static pxr::UsdStageRefPtr compute_WriteFileNode(PyObject *nodeArgs)
{
  std::cout << "WriteFileNode" << std::endl;
  return NULL;
}

static pxr::UsdStageRefPtr compute_MergeNode(PyObject *nodeArgs)
{
  std::cout << "MergeNode" << std::endl;
  return NULL;
}

static pxr::UsdStageRefPtr compute_FilterNode(PyObject *nodeArgs)
{
  std::cout << "FilterNode" << std::endl;
  return NULL;
}

static pxr::UsdStageRefPtr compute_RootNode(PyObject *nodeArgs)
{
  std::cout << "RootNode" << std::endl;
  return NULL;
}

static pxr::UsdStageRefPtr compute_InstancingNode(PyObject *nodeArgs)
{
  std::cout << "InstancingNode" << std::endl;
  return NULL;
}

static pxr::UsdStageRefPtr compute_TransformNode(PyObject *nodeArgs)
{
  std::cout << "TransformNode" << std::endl;
  return NULL;
}

static pxr::UsdStageRefPtr compute_TransformByEmptyNode(PyObject *nodeArgs)
{
  std::cout << "TransformByEmptyNode" << std::endl;
  return NULL;
}

static PyObject *compute(PyObject *self, PyObject *args)
{
  USDNodeType usdNodeType;
  PyObject *nodeArgs;

  if (!PyArg_ParseTuple(args, "iO", &usdNodeType, &nodeArgs)) {
    Py_RETURN_NONE;
  }
  
  pxr::UsdStageRefPtr stage = nullptr;

  //PointerRNA nodeptr;
  //RNA_pointer_create(NULL, &RNA_Node, (void *)PyLong_AsVoidPtr(pynode), &nodeptr);
  //BL::Node node(nodeptr);

  switch (usdNodeType) {
    case USDNodeType::BlenderDataNode:
      stage = compute_BlenderDataNode(nodeArgs);
      break;

    case USDNodeType::UsdFileNode:
      stage = compute_UsdFileNode(nodeArgs);
      break;

    case USDNodeType::HydraRenderNode:
      stage = compute_HydraRenderNode(nodeArgs);
      break;

    case USDNodeType::WriteFileNode:
      stage = compute_WriteFileNode(nodeArgs);
      break;

    case USDNodeType::MergeNode:
      stage = compute_MergeNode(nodeArgs);
      break;

    case USDNodeType::FilterNode:
      stage = compute_FilterNode(nodeArgs);
      break;

    case USDNodeType::RootNode:
      stage = compute_RootNode(nodeArgs);
      break;

    case USDNodeType::InstancingNode:
      stage = compute_InstancingNode(nodeArgs);
      break;

    case USDNodeType::TransformNode:
      stage = compute_TransformNode(nodeArgs);
      break;

    case USDNodeType::TransformByEmptyNode:
      stage = compute_TransformByEmptyNode(nodeArgs);
      break;
  }

  return PyLong_FromVoidPtr(stage.operator->());
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
