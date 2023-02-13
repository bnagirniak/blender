/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <chrono>
#include <string>

#include <pxr/base/gf/matrix4d.h>

namespace blender::render::hydra {

pxr::GfMatrix4d gf_matrix_from_transform(float m[4][4]);
std::string format_duration(std::chrono::milliseconds secs);

} // namespace blender::render::hydra
