/*
 * parser.hpp
 *
 *  Created on: 23 окт. 2019 г.
 *      Author: alexeiakimov
 */

#ifndef PARSER_HPP_
#define PARSER_HPP_

#include <stdint.h>
#include <cstddef>
#include <memory>
#include <vector>
#include "utils.hpp"
#include "writer.hpp"

namespace http_loader {

class ParserBase {
 public:
  // Returns chunk of memory to which input will be stored
  //
  virtual Chunk next() = 0;
  virtual void process(Chunk chunk) = 0;
  virtual bool succeed() = 0;
};

class HttpParser : public ParserBase {
 public:
  HttpParser(std::unique_ptr<FileWriter> writer);
  Chunk next();
  void process(Chunk chunk);
  bool succeed();
  std::string createHttpRequest(std::string_view &host, std::string_view &path);
  ssize_t parseHeader();

 private:
  enum class State {
    BEGIN,
    HEADER_PARSING,
    HEADER_PARSED,
    DOWNLOADING_CONTENT,
    ERROR,
    FINISHED
  };

  State state = State::BEGIN;
  // Use vector just as holder of memory buffer for simplicity
  //
  std::vector<uint8_t> data_holder;

  uint8_t *getBuffer() { return (uint8_t *)data_holder.data() + bufferSize; }

  size_t bufferSize = 0;
  size_t write_treshold = 131072;
  static const size_t bufferCapacity = 135168;  // 128kb + 4096;
  static const size_t header_max_length = 4096;
  std::unique_ptr<FileWriter> writer;
  size_t cont_len = 0;

 public:
  ~HttpParser();
};
}  // end namespace http_loader

#endif /* PARSER_HPP_ */
