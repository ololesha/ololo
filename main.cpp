#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <string_view>
#include "connection.hpp"
#include "parser.hpp"
#include "utils.hpp"
#include "writer.hpp"

namespace http_loader {
bool processLoad(ParserBase *parser, TcpConnection *connection) {
  while (auto chunk = parser->next()) {
    ssize_t recieved = connection->receive(chunk);
    parser->process(Chunk{chunk.ptr, recieved});
  }
  if (parser->succeed()) return true;
  return false;
}
}  // end namespace http_loader

int main(int argc, char *argv[]) {
  using namespace http_loader;

  if (argc != 2) {
    LogError("Usage: http_loader url_for_download");
    exit(EXIT_SUCCESS);
  }

  const char *url = argv[1];

  auto may_be_parsed_url = parse_url(argv[1]);

  if (!may_be_parsed_url) {
    exit(EXIT_FAILURE);
  }
  auto parsed_url = may_be_parsed_url.value();

  auto host = gethostbyname(std::string(parsed_url.addr).data());
  if (!host) {
    LogError("Can't resolv address %s\n", parsed_url.addr);
    exit(EXIT_FAILURE);
  }

  if (host->h_addrtype != AF_INET || !host->h_addr_list[0]) {
    LogError("Can't resolve address %s\n", parsed_url.addr);
    exit(EXIT_FAILURE);
  }

  auto ip = inet_ntoa(*((struct in_addr *)host->h_addr_list[0]));

  if (!ip) {
    LogError("Can't resolve address %s\n", parsed_url.addr);
    exit(EXIT_FAILURE);
  }

  auto tcp_connection = CreateTcpConnection(ip, parsed_url.port);

  if (!tcp_connection) {
    LogError("Can't connect to %s:%d\n", ip, parsed_url.port);
    exit(EXIT_FAILURE);
  }

  auto writer = std::make_unique<FileWriter>(std::string(parsed_url.path));
  auto parser = std::make_unique<HttpParser>(std::move(writer));
  auto request = parser->createHttpRequest(parsed_url.addr, parsed_url.path);
  if (request.empty()) {
    LogError("Can't create http request from path:%s host:%s", parsed_url.path,
             parsed_url.addr);
    exit(EXIT_FAILURE);
  }

  auto sent =
      tcp_connection->send(Chunk{(uint8_t *)request.data(), request.size()});
  if (sent <= 0) {
    LogError("Can't send request to host : %s ", parsed_url.addr);
    exit(EXIT_FAILURE);
  }

  processLoad(parser.get(), tcp_connection.get());

  exit(EXIT_SUCCESS);
}
