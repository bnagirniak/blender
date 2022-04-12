/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <Python.h>

#include "hdusd_python_api.h"

static PyObject *init_func(PyObject * /*self*/, PyObject *args)
{
  Py_RETURN_NONE;
}

static PyObject *exit_func(PyObject * /*self*/, PyObject * /*args*/)
{
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

PyObject *HdUSD_initPython(void)
{
  PyObject *mod = PyModule_Create(&module);
  return mod;
}
