/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <filesystem>
#include <string>
#include <fstream>
#include <process.h>

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

using namespace std;

namespace hdusd {

string get_random_string(const int len);
filesystem::path get_temp_dir(void);
filesystem::path get_temp_pid_dir(void);
string get_temp_file(string suffix, string name = "", bool is_rand = false);

} // namespace hdusd
