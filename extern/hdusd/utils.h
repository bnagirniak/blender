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

std::string get_random_string(const int len);
std::filesystem::path get_temp_dir(void);
std::filesystem::path get_temp_pid_dir(void);
std::string get_temp_file(std::string suffix, std::string name = "", bool is_rand = false);

float getDeterminant(const vector<vector<float>> vect);

vector<vector<float>> getTranspose(const vector<vector<float>> matrix1);

vector<vector<float>> getCofactor(const vector<vector<float>> vect);

vector<vector<float>> getInverse(const vector<vector<float>> vect);

vector<vector<float>> convert_array_4x4_to_vector(BL::Array<float, 16> input_array);

BL::Array<float, 16> convert_vector_to_array_4x4(vector<vector<float>> input_vector);

} // namespace hdusd
