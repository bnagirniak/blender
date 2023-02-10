/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <sstream>

#include "DNA_camera_types.h"

#include "utils.h"

using namespace std;
using namespace pxr;

namespace blender::render::hydra {

GfMatrix4d gf_matrix_from_transform(float m[4][4])
{
  return GfMatrix4d(
    m[0][0], m[0][1], m[0][2], m[0][3],
    m[1][0], m[1][1], m[1][2], m[1][3],
    m[2][0], m[2][1], m[2][2], m[2][3],
    m[3][0], m[3][1], m[3][2], m[3][3]);
}

GfCamera gf_camera_from_camera_object(Object *camera_obj, GfVec2i resolution, GfVec4f tile)
{
  Camera *camera = (Camera *)camera_obj->data;
  float ratio = (float)resolution[0] / resolution[1];

  GfCamera gfCamera;
  gfCamera.SetProjection((camera->type == CAM_ORTHO) ? GfCamera::Orthographic :
                                                       GfCamera::Perspective);
  gfCamera.SetClippingRange(GfRange1f(camera->clip_start, camera->clip_end));
  gfCamera.SetHorizontalAperture(camera->sensor_x);
  gfCamera.SetVerticalAperture(camera->sensor_y / ratio);
  gfCamera.SetFocalLength(camera->lens);

  gfCamera.SetTransform(gf_matrix_from_transform(camera_obj->object_to_world));

  /* TODO: include tile */
  /* TODO: include shifts */
  /* TODO: check orthographic */

  return gfCamera;
}

string format_duration(chrono::milliseconds millisecs)
{
  stringstream ss;
  bool neg = millisecs < 0ms;
  if (neg)
      millisecs = -millisecs;
  auto m = chrono::duration_cast<chrono::minutes>(millisecs);
  millisecs -= m;
  auto s = chrono::duration_cast<chrono::seconds>(millisecs);
  millisecs -= s;
  if (neg)
      ss << "-";
  if (m < 10min)
      ss << "0";
  ss << to_string(m / 1min) << ":";
  if (s < 10s)
      ss << "0";
  ss << to_string(s/1s) << ":";
  if (millisecs < 10ms)
       ss << "0";
  ss << to_string(millisecs/1ms/10);
  return ss.str();
}

} // namespace blender::render::hydra
