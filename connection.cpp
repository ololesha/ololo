/*
 * connection.cpp
 *
 *  Created on: 23 окт. 2019 г.
 *      Author: alexeiakimov
 */

#include "connection.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "utils.hpp"

namespace http_loader {

TcpConnection::TcpConnection(int fd) : socket_fd(fd) {}

std::unique_ptr<TcpConnection> CreateTcpConnection(const char *addr,
                                                   uint16_t port) {
  sockaddr_in sockaddr;
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = inet_addr(addr);
  sockaddr.sin_port = htons(port);

  auto sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    LogError("Socket creation failed");
    return std::unique_ptr<TcpConnection>{};
  }

  struct timeval timeout;
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                 sizeof(timeout)) < 0) {
    LogError("setsockopt failed");
    return std::unique_ptr<TcpConnection>{};
  }

  if (setsockopt(sock_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                 sizeof(timeout)) < 0) {
    LogError("setsockopt failed");
    return std::unique_ptr<TcpConnection>{};
  }

  if (connect(sock_fd, (const struct sockaddr *)&sockaddr, sizeof(sockaddr))) {
    LogError("can not connect to server");
    return std::unique_ptr<TcpConnection>{};
  }
  // Loginfo("Connection to %s:%d established", *addr, port);
  return std::make_unique<TcpConnection>(sock_fd);
}

ssize_t TcpConnection::receive(Chunk chunk) {
  fd_set rfds;
  struct timeval tv;
  tv.tv_sec = read_timeout;
  tv.tv_usec = 0;

  FD_ZERO(&rfds);
  FD_SET(socket_fd, &rfds);

  int result = select(socket_fd + 1, &rfds, NULL, NULL, &tv);
  if (!result) {
    LogError("Timeout reached on select");
    return -1;
  }

  if (result == -1) {
    LogError("Select error");
    return -1;
  }

  ssize_t num_readed = recv(socket_fd, chunk.ptr, chunk.size, MSG_DONTWAIT);
  if (num_readed < 0) {
    LogError("recv error");
    return 0;
  }

  return num_readed;
}

ssize_t TcpConnection::send(Chunk chunk) {
  ssize_t total_sent = 0;
  while (total_sent != chunk.size) {
    ssize_t sent_once =
        ::send(socket_fd, chunk.ptr + total_sent, chunk.size - total_sent, 0);
    if (sent_once < 0) {
      LogError("send error");
      continue;
    }
    total_sent += sent_once;
  }
  return total_sent;
}

}  // end namespace http_loader
