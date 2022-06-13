/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include "utils.h"

namespace hdusd {

string hdusd::get_random_string(const int len)
{
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  string tmp_s;
  tmp_s.reserve(len);

  for (int i = 0; i < len; ++i) {
      tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
  }
    
  return tmp_s;
}

filesystem::path hdusd::get_temp_dir(void)
{
  auto path = filesystem::temp_directory_path() / "hdusd";

  if (!filesystem::exists(path)) {
    filesystem::create_directory(path);
  }

  return path;
}

filesystem::path hdusd::get_temp_pid_dir(void)
{
  filesystem::path path = hdusd::get_temp_dir() / to_string(getpid());

  if (!filesystem::exists(path)) {
    filesystem::create_directory(path);
  }

  return path;
}

string hdusd::get_temp_file(string suffix, string name, bool is_rand)
{
  auto filename = hdusd::get_random_string(8);
  string path;
  if (name.empty()) {
    path = get_temp_pid_dir().u8string() + "/tmp" + filename + suffix;
    ofstream(path.c_str());

    return path;
  }

  if (!suffix.empty()) {
    if (is_rand) {
      path = get_temp_pid_dir().u8string() + "/" + name + "_" + filename + suffix;
      ofstream(path.c_str());

      return path;
    }

    name += suffix;
  }

  return get_temp_pid_dir().u8string() + "/" + name;
}

float hdusd::matrix::get_determinant(const vector<vector<float>> input_vector)
{
    if(input_vector.size() != input_vector[0].size()) {
        throw runtime_error("Matrix is not quadratic");
    } 
    int dimension = input_vector.size();

    if(dimension == 0) {
        return 1;
    }

    if(dimension == 1) {
        return input_vector[0][0];
    }

    if(dimension == 2) {
        return input_vector[0][0] * input_vector[1][1] - input_vector[0][1] * input_vector[1][0];
    }

    float result = 0;
    int sign = 1;
    for(int i = 0; i < dimension; i++) {

        //Submatrix
        vector<vector<float>> subVect(dimension - 1, vector<float> (dimension - 1));
        for(int m = 1; m < dimension; m++) {
            int z = 0;
            for(int n = 0; n < dimension; n++) {
                if(n != i) {
                    subVect[m-1][z] = input_vector[m][n];
                    z++;
                }
            }
        }

        //recursive call
        result = result + sign * input_vector[0][i] * get_determinant(subVect);
        sign = -sign;
    }

    return result;
}

vector<vector<float>> hdusd::matrix::get_transpose(const vector<vector<float>> input_vector)
{

    //Transpose-matrix: height = width(matrix), width = height(matrix)
    vector<vector<float>> solution(input_vector[0].size(), vector<float> (input_vector.size()));

    //Filling solution-matrix
    for(size_t i = 0; i < input_vector.size(); i++) {
        for(size_t j = 0; j < input_vector[0].size(); j++) {
            solution[j][i] = input_vector[i][j];
        }
    }
    return solution;
}

vector<vector<float>> hdusd::matrix::get_cofactor(const vector<vector<float>> input_vector)
{
    if(input_vector.size() != input_vector[0].size()) {
        throw runtime_error("Matrix is not quadratic");
    } 

    vector<vector<float>> solution(input_vector.size(), vector<float> (input_vector.size()));
    vector<vector<float>> subVect(input_vector.size() - 1, vector<float> (input_vector.size() - 1));

    for(size_t i = 0; i < input_vector.size(); i++) {
        for(size_t j = 0; j < input_vector[0].size(); j++) {

            int p = 0;
            for(size_t x = 0; x < input_vector.size(); x++) {
                if(x == i) {
                    continue;
                }
                int q = 0;

                for(size_t y = 0; y < input_vector.size(); y++) {
                    if(y == j) {
                        continue;
                    }

                    subVect[p][q] = input_vector[x][y];
                    q++;
                }
                p++;
            }
            solution[i][j] = pow(-1, i + j) * hdusd::matrix::get_determinant(subVect);
        }
    }
    return solution;
}

vector<vector<float>> hdusd::matrix::get_inverse(const vector<vector<float>> input_vector)
{
    if(hdusd::matrix::get_determinant(input_vector) == 0) {
        throw runtime_error("Determinant is 0");
    } 

    float d = 1.0/hdusd::matrix::get_determinant(input_vector);
    vector<vector<float>> solution(input_vector.size(), vector<float> (input_vector.size()));

    for(size_t i = 0; i < input_vector.size(); i++) {
        for(size_t j = 0; j < input_vector.size(); j++) {
            solution[i][j] = input_vector[i][j]; 
        }
    }

    solution = hdusd::matrix::get_transpose(get_cofactor(solution));

    for(size_t i = 0; i < input_vector.size(); i++) {
        for(size_t j = 0; j < input_vector.size(); j++) {
            solution[i][j] *= d;
        }
    }

    return solution;
}

vector<vector<float>> hdusd::matrix::convert_array_4x4_to_vector(BL::Array<float, 16> input_array)
{
  return {{input_array.data[0], input_array.data[1], input_array.data[2], input_array.data[3]}, 
          {input_array.data[4], input_array.data[5], input_array.data[6], input_array.data[7]},
          {input_array.data[8], input_array.data[9], input_array.data[10], input_array.data[11]},
          {input_array.data[12], input_array.data[13], input_array.data[14], input_array.data[15]}};
};

BL::Array<float, 16> hdusd::matrix::convert_vector_to_array_4x4(vector<vector<float>> input_vector)
{
  BL::Array<float, 16> arr;
  int i = 0;
  for(auto& row:input_vector){
     for(auto& col:row){
        arr[i] = col;
        i++;
     }
  }
  return arr;
}

} // namespace hdusd
