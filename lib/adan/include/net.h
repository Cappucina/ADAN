#ifndef ADAN_NET_H
#define ADAN_NET_H

#include <stddef.h>

int tcp_listen(int port);

int tcp_accept(int server_fd);

int tcp_connect(const char* ip, int port);

int tcp_read(int fd, void* buffer, size_t max_len);

int tcp_write(int fd, const void* buffer, size_t len);

void tcp_close(int fd);

int tcp_read_request(int fd, void* buffer, size_t max_len);

int tcp_write_all(int fd, const void* buffer, size_t len);

char* tcp_read_request_str(int fd);

int tcp_write_string(int fd, const char* s);

#endif
