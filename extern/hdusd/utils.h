/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <filesystem>
#include <string>
#include <fstream>
#include <process.h>

namespace hdusd {

std::string get_random_string(const int len);

std::filesystem::path get_temp_dir(void);

std::filesystem::path get_temp_pid_dir(void);

std::string get_temp_file(std::string suffix, std::string name = "", bool is_rand = false);

} // namespace hdusd
