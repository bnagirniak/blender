/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <Python.h>

#include <pxr/pxr.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/stageCache.h>

namespace hdusd {

extern std::unique_ptr<pxr::UsdStageCache> stageCache;

PyObject *usd_addPythonSubmodule(PyObject *mod);

} // namespace hdusd
