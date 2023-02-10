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
