/*
 * utils.hpp
 *
 *  Created on: 23 окт. 2019 г.
 *      Author: alexeiakimov
 */

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <stdarg.h>
#include <stdio.h>
#include <optional>
#include <string_view>

namespace http_loader {

struct Chunk {
  uint8_t* ptr = nullptr;
  size_t size = 0;
  operator bool() { return ptr != nullptr && size != 0; }
};

class WriteWrapper {
 public:
  WriteWrapper(const std::string& path);
  int write();

 private:
  FILE* fd = NULL;
  std::string path;
};

struct ParsedUrl {
  std::string_view addr;
  std::string_view path;
  uint16_t port;
};

std::optional<ParsedUrl> parse_url(std::string_view url);

static void LogError(const char* format, ...) {
  va_list args;
  fprintf(stderr, "Error: ");
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, "\n");
}

static void LogInfo(const char* format, ...) {
  va_list args;
  fprintf(stderr, "Info: ");
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, "\n");
}

}  // end namespace http_loader
#endif /* UTILS_HPP_ */
