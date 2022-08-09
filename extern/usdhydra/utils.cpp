/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <boost/random.hpp>

#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/xform.h>

#include "utils.h"
#include "stage.h"

namespace usdhydra {

string get_random_string(const int len)
{
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  static boost::random::mt19937 rng;
  static boost::random::uniform_int_distribution<> distribution(0, 63);

  string tmp_s;
  tmp_s.reserve(len);

  for (int i = 0; i < len; ++i) {
      tmp_s += alphanum[distribution(rng) % (sizeof(alphanum) - 1)];
  }
    
  return tmp_s;
}

filesystem::path get_temp_dir(void)
{
  auto path = filesystem::temp_directory_path() / "usdhydra";

  if (!filesystem::exists(path)) {
    filesystem::create_directory(path);
  }

  return path;
}

filesystem::path get_temp_pid_dir(void)
{
  filesystem::path path = get_temp_dir() / to_string(getpid());

  if (!filesystem::exists(path)) {
    filesystem::create_directory(path);
  }

  return path;
}

string get_temp_file(string suffix, string name, bool is_rand)
{
  auto filename = get_random_string(8);
  string path;
  if (name.empty()) {
    path = get_temp_pid_dir().u8string() + "/tmp" + filename + suffix;
    ofstream(path.c_str());

    return path;
  }

  if (!suffix.empty()) {
    if (is_rand) {
      path = get_temp_pid_dir().u8string() + "/" + name + "_" + filename + suffix;
      ofstream(path.c_str());

      return path;
    }

    name += suffix;
  }

  return get_temp_pid_dir().u8string() + "/" + name;
}

string format_milliseconds(chrono::milliseconds millisecs)
{
    bool neg = millisecs < 0ms;
    if (neg)
        millisecs = -millisecs;
    auto m = chrono::duration_cast<chrono::minutes>(millisecs);
    millisecs -= m;
    auto s = chrono::duration_cast<chrono::seconds>(millisecs);
    millisecs -= s;
    std::string result;
    if (neg)
        result.push_back('-');
    if (m < 10min)
        result.push_back('0');
    result += to_string(m/1min);
    result += ':';
    if (s < 10s)
        result.push_back('0');
    result += to_string(s/1s);
    result += ':';
    if (millisecs < 10ms)
        result.push_back('0');
    result += to_string(millisecs/1ms/10);
    return result;
}

static PyObject *get_temp_file_func(PyObject * /*self*/, PyObject *args)
{
  const char *suffix = "", *name = "";
  bool is_rand = false;

  PyArg_ParseTuple(args, "ssp", &suffix, &name, &is_rand);

  filesystem::path path = usdhydra::get_temp_file(suffix, name, is_rand);
  return PyUnicode_FromString(path.u8string().c_str());
}

static PyObject *get_temp_dir_func(PyObject * /*self*/, PyObject * /*args*/)
{
  filesystem::path path = usdhydra::get_temp_dir();
  return PyUnicode_FromString(path.u8string().c_str());
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

  auto matrix = (double(*)[4][4])transform.GetArray();

  PyObject *pyMatrix = PyTuple_New(4);

  PyObject *pyRow_0 = PyTuple_New(4);
  PyObject *pyRow_1 = PyTuple_New(4);
  PyObject *pyRow_2 = PyTuple_New(4);
  PyObject *pyRow_3 = PyTuple_New(4);

  PyTuple_SetItem(pyRow_0, 0, PyFloat_FromDouble((*matrix)[0][0]));
  PyTuple_SetItem(pyRow_0, 1, PyFloat_FromDouble((*matrix)[0][1]));
  PyTuple_SetItem(pyRow_0, 2, PyFloat_FromDouble((*matrix)[0][2]));
  PyTuple_SetItem(pyRow_0, 3, PyFloat_FromDouble((*matrix)[0][3]));
                                                         
  PyTuple_SetItem(pyRow_1, 0, PyFloat_FromDouble((*matrix)[1][0]));
  PyTuple_SetItem(pyRow_1, 1, PyFloat_FromDouble((*matrix)[1][1]));
  PyTuple_SetItem(pyRow_1, 2, PyFloat_FromDouble((*matrix)[1][2]));
  PyTuple_SetItem(pyRow_1, 3, PyFloat_FromDouble((*matrix)[1][3]));
                                                         
  PyTuple_SetItem(pyRow_2, 0, PyFloat_FromDouble((*matrix)[2][0]));
  PyTuple_SetItem(pyRow_2, 1, PyFloat_FromDouble((*matrix)[2][1]));
  PyTuple_SetItem(pyRow_2, 2, PyFloat_FromDouble((*matrix)[2][2]));
  PyTuple_SetItem(pyRow_2, 3, PyFloat_FromDouble((*matrix)[2][3]));
                                                         
  PyTuple_SetItem(pyRow_3, 0, PyFloat_FromDouble((*matrix)[3][0]));
  PyTuple_SetItem(pyRow_3, 1, PyFloat_FromDouble((*matrix)[3][1]));
  PyTuple_SetItem(pyRow_3, 2, PyFloat_FromDouble((*matrix)[3][2]));
  PyTuple_SetItem(pyRow_3, 3, PyFloat_FromDouble((*matrix)[3][3]));

  PyTuple_SetItem(pyMatrix, 0, pyRow_0);
  PyTuple_SetItem(pyMatrix, 1, pyRow_1);
  PyTuple_SetItem(pyMatrix, 2, pyRow_2);
  PyTuple_SetItem(pyMatrix, 3, pyRow_3);

  return pyMatrix;
}


static PyMethodDef methods[] = {
  {"get_temp_file", get_temp_file_func, METH_VARARGS, ""},
  {"get_temp_dir", get_temp_dir_func, METH_VARARGS, ""},
  {"get_xform_transform", get_xform_transform_func, METH_VARARGS, ""},
  {NULL, NULL, 0, NULL},
};

static struct PyModuleDef module = {
  PyModuleDef_HEAD_INIT,
  "utils",
  "This module provides access to utils functions.",
  -1,
  methods,
  NULL,
  NULL,
  NULL,
  NULL,
};

PyObject *addPythonSubmodule_utils(PyObject *mod)
{
  PyObject *submodule = PyModule_Create(&module);
  PyModule_AddObject(mod, "utils", submodule);
  return submodule;
}

} // namespace usdhydra
