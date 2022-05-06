/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <Python.h>
#include <iostream>
#include <string.h>
#include <pxr/pxr.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/base/tf/stringUtils.h>

#include <pxr/usd/usd/stage.h>

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

#include "hdusd_python_api.h"
#include "session.h"
#include "utils.h"

enum class USDNodeType
{
  BlenderDataNode,
  UsdFileNode,
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
  char *filePath, *filterPath;
  PyArg_ParseTuple(nodeArgs, "ss", &filePath, &filterPath);
  return pxr::UsdStage::Open(filePath);
  
        //if self.filter_path == '/*':
        //    self.cached_stage.insert(input_stage)
        //    return input_stage

        //# creating search regex pattern and getting filtered rpims
        //prog = re.compile(self.filter_path.replace('*', '#')        # temporary replacing '*' to '#'
        //                  .replace('/', '\/')       # for correct regex pattern
        //                  .replace('##', '[\w\/]*') # creation
        //                  .replace('#', '\w*'))

        //def get_child_prims(prim):
        //    if not prim.IsPseudoRoot() and prog.fullmatch(str(prim.GetPath())):
        //        yield prim
        //        return

        //    for child in prim.GetAllChildren():
        //        yield from get_child_prims(child)

        //prims = tuple(get_child_prims(input_stage.GetPseudoRoot()))
        //if not prims:
        //    return None

        //stage = self.cached_stage.create()
        //stage.SetInterpolationType(Usd.InterpolationTypeHeld)
        //UsdGeom.SetStageMetersPerUnit(stage, 1)
        //UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.z)

        //root_prim = stage.GetPseudoRoot()
        //for i, prim in enumerate(prims, 1):
        //    override_prim = stage.OverridePrim(root_prim.GetPath().AppendChild(prim.GetName()))
        //    override_prim.GetReferences().AddReference(input_stage.GetRootLayer().realPath, prim.GetPath())

        //return stage
}

static pxr::UsdStageRefPtr compute_MergeNode(PyObject *nodeArgs)
{
        //stage = self.cached_stage.create()
        //UsdGeom.SetStageMetersPerUnit(stage, 1)
        //UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.z)

        //root_prim = stage.GetPseudoRoot()

        //for ref_stage in ref_stages:
        //    for prim in ref_stage.GetPseudoRoot().GetAllChildren():
        //        override_prim = stage.OverridePrim(root_prim.GetPath().AppendChild(prim.GetName()))
        //        override_prim.GetReferences().AddReference(ref_stage.GetRootLayer().realPath, prim.GetPath())

        //return stage

  std::cout << "MergeNode" << std::endl;
  return NULL;
}

static pxr::UsdStageRefPtr compute_FilterNode(PyObject *nodeArgs)
{
        //# creating search regex pattern and getting filtered rpims
        //prog = re.compile(self.filter_path.replace('*', '#')        # temporary replacing '*' to '#'
        //                                  .replace('/', '\/')       # for correct regex pattern
        //                                  .replace('##', '[\w\/]*') # creation
        //                                  .replace('#', '\w*'))

        //def get_child_prims(prim):
        //    if not prim.IsPseudoRoot() and prog.fullmatch(str(prim.GetPath())):
        //        yield prim
        //        return

        //    for child in prim.GetAllChildren():
        //        yield from get_child_prims(child)

        //prims = tuple(get_child_prims(input_stage.GetPseudoRoot()))
        //if not prims:
        //    return None

        //stage = self.cached_stage.create()
        //UsdGeom.SetStageMetersPerUnit(stage, 1)
        //UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.z)

        //root_prim = stage.GetPseudoRoot()

        //for i, prim in enumerate(prims, 1):
        //    override_prim = stage.OverridePrim(root_prim.GetPath().AppendChild(prim.GetName()))
        //    override_prim.GetReferences().AddReference(input_stage.GetRootLayer().realPath,
        //                                               prim.GetPath())

        //return stage

  std::cout << "FilterNode" << std::endl;
  return NULL;
}

static pxr::UsdStageRefPtr compute_RootNode(PyObject *nodeArgs)
{
        //path = f'/{Tf.MakeValidIdentifier(self.name)}'
        //stage = self.cached_stage.create()
        //UsdGeom.SetStageMetersPerUnit(stage, 1)
        //UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.z)

        //# create new root prim according to name and type
        //if self.type == 'Xform':
        //    root_prim = UsdGeom.Xform.Define(stage, path)
        //elif self.type == 'Scope':
        //    root_prim = UsdGeom.Scope.Define(stage, path)
        //elif self.type == 'SkelRoot':
        //    root_prim = UsdSkel.Root.Define(stage, path)
        //else:
        //    root_prim = stage.DefinePrim(path)

        //for prim in input_stage.GetPseudoRoot().GetAllChildren():
        //    override_prim = stage.OverridePrim(root_prim.GetPath().AppendChild(prim.GetName()))
        //    override_prim.GetReferences().AddReference(input_stage.GetRootLayer().realPath, prim.GetPath())

        //return stage

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

  //std::string name = pxr::TfMakeValidIdentifier(node->name);

  //std::string path = hdusd_utils::get_temp_dir().u8string();
  std::string path = hdusd::get_temp_file(".usda");
  std::cout << path << std::endl;
  //pxr::UsdStageRefPtr stage = pxr::UsdStage::CreateNew();

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

  if (!stage) {
    Py_RETURN_NONE;
  }

  auto id = stageCache->Insert(stage);

  std::string str;
  stage->ExportToString(&str);
  std::cout << str <<std::endl;

  return PyLong_FromLong(id.ToLongInt());
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
  PyModule_AddObject(usdNodeTypeMod, "MergeNode", PyLong_FromLong((int)USDNodeType::MergeNode));
  PyModule_AddObject(usdNodeTypeMod, "FilterNode", PyLong_FromLong((int)USDNodeType::FilterNode));
  PyModule_AddObject(usdNodeTypeMod, "RootNode", PyLong_FromLong((int)USDNodeType::RootNode));
  PyModule_AddObject(usdNodeTypeMod, "InstancingNode", PyLong_FromLong((int)USDNodeType::InstancingNode));
  PyModule_AddObject(usdNodeTypeMod, "TransformNode", PyLong_FromLong((int)USDNodeType::TransformNode));
  PyModule_AddObject(usdNodeTypeMod, "TransformByEmptyNode", PyLong_FromLong((int)USDNodeType::TransformByEmptyNode));

  PyModule_AddObject(mod, "type", usdNodeTypeMod);

  return mod;
}
