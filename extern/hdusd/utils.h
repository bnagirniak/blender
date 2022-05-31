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

namespace matrix {

float get_determinant(const vector<vector<float>> input_vector);
vector<vector<float>> get_transpose(const vector<vector<float>> input_vector);
vector<vector<float>> get_cofactor(const vector<vector<float>> input_vector);
vector<vector<float>> get_inverse(const vector<vector<float>> input_vector);

vector<vector<float>> convert_array_4x4_to_vector(BL::Array<float, 16> input_array);
BL::Array<float, 16> convert_vector_to_array_4x4(vector<vector<float>> input_vector);

}

} // namespace hdusd
