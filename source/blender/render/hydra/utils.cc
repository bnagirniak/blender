/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <sstream>

#include "utils.h"

using namespace std;

namespace blender::render::hydra {

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
