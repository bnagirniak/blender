/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include "utils.h"

namespace hdusd {

string get_random_string(const int len)
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

filesystem::path get_temp_dir(void)
{
  auto path = filesystem::temp_directory_path() / "hdusd";

  if (!filesystem::exists(path)) {
    filesystem::create_directory(path);
  }

  return path;
}

filesystem::path get_temp_pid_dir(void)
{
  filesystem::path path = get_temp_dir() / to_string(getpid());

  if (!filesystem::exists(path)) {
    filesystem::create_directory(path);
  }

  return path;
}

string get_temp_file(string suffix, string name, bool is_rand)
{
  auto filename = get_random_string(8);
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

float getDeterminant(const vector<vector<float>> vect) {
    if(vect.size() != vect[0].size()) {
        throw runtime_error("Matrix is not quadratic");
    } 
    int dimension = vect.size();

    if(dimension == 0) {
        return 1;
    }

    if(dimension == 1) {
        return vect[0][0];
    }

    if(dimension == 2) {
        return vect[0][0] * vect[1][1] - vect[0][1] * vect[1][0];
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
                    subVect[m-1][z] = vect[m][n];
                    z++;
                }
            }
        }

        //recursive call
        result = result + sign * vect[0][i] * getDeterminant(subVect);
        sign = -sign;
    }

    return result;
}

vector<vector<float>> getTranspose(const vector<vector<float>> matrix1) {

    //Transpose-matrix: height = width(matrix), width = height(matrix)
    vector<vector<float>> solution(matrix1[0].size(), vector<float> (matrix1.size()));

    //Filling solution-matrix
    for(size_t i = 0; i < matrix1.size(); i++) {
        for(size_t j = 0; j < matrix1[0].size(); j++) {
            solution[j][i] = matrix1[i][j];
        }
    }
    return solution;
}

vector<vector<float>> getCofactor(const vector<vector<float>> vect) {
    if(vect.size() != vect[0].size()) {
        throw runtime_error("Matrix is not quadratic");
    } 

    vector<vector<float>> solution(vect.size(), vector<float> (vect.size()));
    vector<vector<float>> subVect(vect.size() - 1, vector<float> (vect.size() - 1));

    for(size_t i = 0; i < vect.size(); i++) {
        for(size_t j = 0; j < vect[0].size(); j++) {

            int p = 0;
            for(size_t x = 0; x < vect.size(); x++) {
                if(x == i) {
                    continue;
                }
                int q = 0;

                for(size_t y = 0; y < vect.size(); y++) {
                    if(y == j) {
                        continue;
                    }

                    subVect[p][q] = vect[x][y];
                    q++;
                }
                p++;
            }
            solution[i][j] = pow(-1, i + j) * getDeterminant(subVect);
        }
    }
    return solution;
}

vector<vector<float>> getInverse(const vector<vector<float>> vect) {
    if(getDeterminant(vect) == 0) {
        throw runtime_error("Determinant is 0");
    } 

    float d = 1.0/getDeterminant(vect);
    vector<vector<float>> solution(vect.size(), vector<float> (vect.size()));

    for(size_t i = 0; i < vect.size(); i++) {
        for(size_t j = 0; j < vect.size(); j++) {
            solution[i][j] = vect[i][j]; 
        }
    }

    solution = getTranspose(getCofactor(solution));

    for(size_t i = 0; i < vect.size(); i++) {
        for(size_t j = 0; j < vect.size(); j++) {
            solution[i][j] *= d;
        }
    }

    return solution;
}

vector<vector<float>> convert_array_4x4_to_vector(BL::Array<float, 16> input_array) {
  vector<float> v1{input_array.data[0], input_array.data[1], input_array.data[2], input_array.data[3]};
  vector<float> v2{input_array.data[4], input_array.data[5], input_array.data[6], input_array.data[7]};
  vector<float> v3{input_array.data[8], input_array.data[9], input_array.data[10], input_array.data[11]};
  vector<float> v4{input_array.data[12], input_array.data[13], input_array.data[14], input_array.data[15]};

  return {v1, v2, v3, v4};
};

BL::Array<float, 16> convert_vector_to_array_4x4(vector<vector<float>> input_vector) {
  //vector<float> vector = {*input_vector[0].data(), *input_vector[1].data(), *input_vector[2].data(), *input_vector[3].data()};
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
