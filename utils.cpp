/*
 * utils.cpp
 *
 *  Created on: 23 окт. 2019 г.
 *      Author: alexeiakimov
 */

#include "utils.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <optional>

namespace http_loader {

std::optional<ParsedUrl> parse_url(std::string_view url) {
  ParsedUrl result;

  if (url.empty()) {
    LogError("Invalid args : empty url");
    return std::nullopt;
  }

  const std::string http_prefix = "http://";

  if (url.find(http_prefix) != 0) {
    LogError("can't parse url");
    return std::nullopt;
  }

  url.remove_prefix(http_prefix.size());

  unsigned long port = 80;
  if (auto colon_iter = url.find(':'); colon_iter != std::string_view::npos) {
    auto address = url.substr(0, colon_iter);

    if (address.empty()) {
      LogError("can't parse url");
      return std::nullopt;
    }

    result.addr = address;
    url.remove_prefix(colon_iter + 1);
    unsigned long port = 80;

    try {
      port = std::stoul(std::string(url), 0, 10);
    } catch (...) {
      LogError("can't parse url");
      return std::nullopt;
    }

    if (port > UINT16_MAX) {
      LogError("can't parse url");
      return std::nullopt;
    }

    auto slash_pos = url.find('/');
    if (slash_pos != std::string_view::npos) {
      url.remove_prefix(slash_pos);
    }

    result.addr = address;
    auto arg_pos = url.find('?');

    if (arg_pos != std::string_view::npos) {
      result.path = url.substr(0, arg_pos - 1);
    } else {
      result.path = url;
    }
    result.port = port;
    return result;
  }

  result.port = port;
  auto slashpos = url.find('/');
  result.addr = url.substr(0, slashpos);
  url.remove_prefix(slashpos);

  result.path = url.substr(0, url.find('?'));
  return result;
}
}  // end namespace http_loader
