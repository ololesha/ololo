/*
 * writer.cpp
 *
 *  Created on: 24 окт. 2019 г.
 *      Author: alexeiakimov
 */
#include "writer.hpp"
#include <unistd.h>
namespace http_loader {

ssize_t FileWriter::write(Chunk chunk) {
  if (!fd) {
    fd = fopen(path.c_str(), "w");
    if (!fd) {
      LogError("Can not open file %s", path);
      return -2;
    }
  }
  return fwrite(chunk.ptr, sizeof(char), chunk.size, fd);
}

FileWriter::FileWriter(const std::string &new_path) {
  auto last_slash = new_path.find_last_of('/');
  if (last_slash != std::string::npos) {
    path = new_path.substr(last_slash + 1);
  } else {
    path = new_path;
  }
}

}  // end namespace http_loader
