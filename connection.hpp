/*
 * connection.hpp
 *
 *  Created on: 23 окт. 2019 г.
 *      Author: alexeiakimov
 */

#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>
#include "parser.hpp"
#include "utils.hpp"

namespace http_loader {

class ConnectionBase {
  virtual ssize_t receive(Chunk) = 0;
  virtual ssize_t send(Chunk) = 0;
};

class TcpConnection : public ConnectionBase {
 public:
  TcpConnection(int socket_fd);
  ssize_t receive(Chunk) override;
  ssize_t send(Chunk) override;

 private:
  int socket_fd = 0;

  // Connection read timeout in seconds
  //
  __time_t read_timeout = 10;
  // No write timeout couse request is sent sinchronously
  //
 public:
  ~TcpConnection() { close(socket_fd); };
};

// Factory function for creating connection
// If something got wrong, just return nullptr
std::unique_ptr<TcpConnection> CreateTcpConnection(const char *addr,
                                                   uint16_t port);
}  // end namespace http_loader

#endif /* CONNECTION_HPP_ */
