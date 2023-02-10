/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <chrono>
#include <string>

#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/vec2i.h>

#include "DNA_object_types.h"

namespace blender::render::hydra {

pxr::GfMatrix4d gf_matrix_from_transform(float m[4][4]);
pxr::GfCamera gf_camera_from_camera_object(Object *camera_obj, pxr::GfVec2i resolution, pxr::GfVec4f tile);
std::string format_duration(std::chrono::milliseconds secs);

} // namespace blender::render::hydra
