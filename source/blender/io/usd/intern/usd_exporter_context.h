/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2019 Blender Foundation. All rights reserved. */
#pragma once

#include "usd.h"

#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/common.h>

struct Depsgraph;
struct Main;

namespace blender::io::usd {

typedef std::map<std::string, std::pair<std::string, std::string>> materialx_data_type;

class USDHierarchyIterator;

struct USDExporterContext {
  Main *bmain;
  Depsgraph *depsgraph;
  const pxr::UsdStageRefPtr stage;
  const pxr::SdfPath usd_path;
  const USDHierarchyIterator *hierarchy_iterator;
  const USDExportParams &export_params;
  const materialx_data_type materialx_data;
};

}  // namespace blender::io::usd
