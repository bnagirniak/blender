/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <chrono>
#include <string>

namespace blender::render::hydra {

std::string formatDuration(std::chrono::milliseconds secs);

} // namespace blender::render::hydra
