/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <Python.h>

#include "material.h"
#include "BKE_material.h"

using namespace pxr;

namespace usdhydra {

MaterialExport::MaterialExport(BL::Object &b_object)
  : material(nullptr)
{
  Object *object = (Object *)b_object.ptr.data;
  if (BKE_object_material_count_eval(object) == 0) {
    return;
  }
    
  material = BKE_object_material_get_eval(object, object->actcol);
}

MaterialExport::MaterialExport(BL::Material& b_material)
  : material((Material *)b_material.ptr.data)
{
}

MaterialExport::operator bool()
{
  return bool(material);
}

std::string MaterialExport::name()
{
  return material->id.name + 2;
}

SdfAssetPath MaterialExport::exportMX()
{
  PyObject *module, *dict, *func, *params, *result;
  module = PyImport_Import(PyUnicode_FromString("usdhydra.matx"));
  dict = PyModule_GetDict(module);
  func = PyDict_GetItemString(dict, "export");
  params = Py_BuildValue("(s)", name().c_str());
  result = PyObject_CallObject(func, params);

  std::string path = PyUnicode_AsUTF8(result);
  return SdfAssetPath(path, path);
}

} // namespace usdhydra
