/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <pxr/base/gf/camera.h>

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

using namespace std;

namespace hdusd {

class CameraData {
  public:
    CameraData();

    ~CameraData();

    static CameraData init_from_camera(BL::Camera b_camera, BL::Array<float, 16> b_transform, float ratio,
                          vector<vector<int>> border = {{0, 0}, {1, 1}});
    static CameraData init_from_context(BL::Context b_context);

    pxr::GfCamera export_gf(vector<float> tile);

    BL::Camera::type_enum mode;
    vector<float> clip_range;
    float focal_length = 0.0;
    vector<float> sensor_size;
    BL::Array<float, 16> transform = {};
    vector<float> lens_shift;
    vector<float> ortho_size;
    tuple<float, float, int> dof_size;
};

} // namespace hdusd
