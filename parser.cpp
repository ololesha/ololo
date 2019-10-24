/*
 * parser.cpp
 *
 *  Created on: 23 окт. 2019 г.
 *      Author: alexeiakimov
 */

#include "parser.hpp"
#include <cstring>

namespace http_loader {

static const int request_size = 1024;

HttpParser::HttpParser(std::unique_ptr<FileWriter> writer)
    : writer(std::move(writer)) {
  data_holder.reserve(bufferCapacity);
};

std::string HttpParser::createHttpRequest(std::string_view& host,
                                          std::string_view& path) {
  if (path.empty() || host.empty()) {
    LogError("Path or host not set");
    return std::string();
  }

  if (path.size() > 200 || host.size() > 200) {
    LogError("Host or path greater 200 charecters");
    return std::string();
  }

  std::string result;
  result.reserve(request_size);

  result = "GET " + std::string(path) + " HTTP/1.1\r\n" +
           "User-Agent: wget_clone \r\n" + "Accept: text/html\r\n" +
           "Accept-Encoding: 0\r\n" + "Connection: Keep-Alive\r\n" +
           "Host: " + std::string(host) + "\r\n\r\n";

  return result;
}

Chunk HttpParser::next() {
  if (state == State::FINISHED) return {nullptr, 0};

  return Chunk{getBuffer(), bufferCapacity - bufferSize};
}

ssize_t HttpParser::parseHeader() {
  std::string_view str((const char*)data_holder.data(), bufferSize);
  auto end = str.find("\r\n\r\n");

  if (end == std::string_view::npos) {
    if (bufferSize > header_max_length) {
      LogError("Http header not found in %d bytes of response",
               header_max_length);
      return -1;
    }
    return 0;
  }

  end += strlen("\r\n\r\n");

  auto header = str.substr(0, end);

  auto code = header.find("HTTP/1.1 ");
  if (code == std::string_view::npos) {
    LogError("Can't find http version in response");
    return -1;
  }

  code += strlen("HTTP/1.1 ");
  unsigned long code_value = strtoul(header.data() + code, NULL, 10);
  if (!code_value) {
    LogError("Can't find http status code");
  }

  auto content_length = header.find("Content-Length: ");
  if (content_length == std::string_view::npos) {
    LogError("Can't find content length");
    return -1;
  }

  auto content_length_ptr =
      header.data() + content_length + strlen("Content-Length: ");
  cont_len = strtoul(content_length_ptr, NULL, 10);
  if (!cont_len) {
    return -1;
  }

  return header.size();
}
HttpParser::~HttpParser(){};

bool HttpParser::succeed() { return state == State::FINISHED; }
void HttpParser::process(Chunk chunk) {
  bufferSize = bufferSize + chunk.size;
  switch (state) {
    case State::BEGIN:
    case State::HEADER_PARSING: {
      state = State::HEADER_PARSING;
      int header_len = parseHeader();
      if (header_len < 0) {
        state = State::ERROR;
        return;
      }
      if (!header_len) {
        state = State::HEADER_PARSING;
        return;
      }
      if (header_len > 0) {
        state = State::DOWNLOADING_CONTENT;
        size_t tail_len = bufferSize - header_len;
        memcpy(data_holder.data(), data_holder.data() + header_len, tail_len);
        bufferSize = 0;
        // For the when we receive whole data in this attempt
        //
        process(Chunk{(uint8_t*)data_holder.data(), tail_len});
        return;
      }
    }
    case State::DOWNLOADING_CONTENT: {
      if (bufferSize == cont_len) {
        Chunk chunk{data_holder.data(), bufferSize};
        writer->write(chunk);
        state = State::FINISHED;
        return;
      }
      if (bufferSize > cont_len) {
        LogError("Received more data then content_lenght");
        state = State::ERROR;
        return;
      }
      if (bufferSize < write_treshold) {
        // Just wait for enough data for write_back
        //
        return;
      }
      // BufferSize >= write_treshold
      //
      size_t tail_size = bufferSize - write_treshold;
      writer->write(Chunk{data_holder.data(), write_treshold});
      memcpy(data_holder.data(), data_holder.data() + write_treshold,
             tail_size);
      cont_len -= write_treshold;
      bufferSize = tail_size;
      return;
    }
  }
  return;
}
}  // end namespace http_loader
