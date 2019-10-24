/*
 * writer.hpp
 *
 *  Created on: 24 окт. 2019 г.
 *      Author: alexeiakimov
 */

#ifndef WRITER_HPP_
#define WRITER_HPP_
#include <sys/types.h>
#include <string>
#include "utils.hpp"

namespace http_loader {

class BaseWriter {
 public:
  virtual ssize_t write(Chunk chunk) = 0;
};

class FileWriter : public BaseWriter {
 public:
  FileWriter(const std::string& path);
  ssize_t write(Chunk chunk);
  virtual ~FileWriter() { fclose(fd); }

 private:
  FILE* fd = NULL;
  std::string path;
};

}  // end namespace http_loader

#endif /* WRITER_HPP_ */
