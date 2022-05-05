/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include "utils.h"

namespace hdusd {

std::string get_random_string(const int len)
{
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  std::string tmp_s;
  tmp_s.reserve(len);

  for (int i = 0; i < len; ++i) {
      tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
  }
    
  return tmp_s;
}

std::filesystem::path get_temp_dir(void)
{
  auto path = std::filesystem::temp_directory_path() / "hdusd";

  if (!std::filesystem::exists(path)) {
    std::filesystem::create_directory(path);
  }

  return path;
}

std::filesystem::path get_temp_pid_dir(void)
{
  std::filesystem::path path = get_temp_dir() / std::to_string(getpid());

  if (!std::filesystem::exists(path)) {
    std::filesystem::create_directory(path);
  }

  return path;
}

std::string get_temp_file(std::string suffix, std::string name, bool is_rand)
{
  auto filename = get_random_string(8);
  std::string path;
  if (name.empty()) {
    path = get_temp_pid_dir().u8string() + "/tmp" + filename + suffix;
    std::ofstream(path.c_str());

    return path;
  }

  if (!suffix.empty()) {
    if (is_rand) {
      path = get_temp_pid_dir().u8string() + "/" + name + "_" + filename + suffix;
      std::ofstream(path.c_str());

      return path;
    }

    name += suffix;
  }

  return get_temp_pid_dir().u8string() + "/" + name;
}

} // namespace hdusd
