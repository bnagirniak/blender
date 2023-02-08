/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <sstream>

#include "DNA_camera_types.h"

#include "utils.h"

using namespace std;
using namespace pxr;

namespace blender::render::hydra {

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

  float *m = (float *)camera_obj->object_to_world;
  gfCamera.SetTransform(GfMatrix4d(
    m[0], m[1], m[2], m[3],
    m[4], m[5], m[6], m[7],
    m[8], m[9], m[10], m[11],
    m[12], m[13], m[14], m[15]));

  /* TODO: include tile */
  /* TODO: include shifts */
  /* TODO: check orthographic */

  return gfCamera;
}

string formatDuration(chrono::milliseconds millisecs)
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
