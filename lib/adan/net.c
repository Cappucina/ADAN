#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "net.h"

// Read until CRLF CRLF ("\r\n\r\n") or until max_len reached.
// Returns number of bytes placed in buffer (excluding terminating '\0'),
// 0 on orderly EOF, or -1 on error.
int tcp_read_request(int fd, void* buffer, size_t max_len) {
	if (!buffer || max_len == 0) return -1;
	char* buf = (char*)buffer;
	size_t pos = 0;

	while (pos + 1 < max_len) {
		ssize_t n = read(fd, buf + pos, 1);
		if (n < 0) {
			if (errno == EINTR) continue;
			return -1;
		}
		if (n == 0) {
			// EOF
			buf[pos] = '\0';
			return pos;
		}
		pos += (size_t)n;
		if (pos >= 4 && buf[pos-4] == '\r' && buf[pos-3] == '\n' && buf[pos-2] == '\r' && buf[pos-1] == '\n') {
			buf[pos] = '\0';
			return pos;
		}
	}

	// buffer full; ensure null-terminated
	buf[max_len-1] = '\0';
	return (int)pos;
}

int tcp_write_all(int fd, const void* buffer, size_t len) {
	if (!buffer) return -1;
	size_t written = 0;
	const char* p = (const char*)buffer;

	while (written < len) {
		ssize_t n = write(fd, p + written, len - written);
		if (n < 0) {
			if (errno == EINTR) continue;
			return -1;
		}
		if (n == 0) return (int)written;
		written += (size_t)n;
	}

	return (int)written;
}

int tcp_listen(int port) {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) return -1;

	int opt = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		close(fd);
		return -1;
	}

	if (listen(fd, 128) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

int tcp_accept(int server_fd) {
	return accept(server_fd, NULL, NULL);
}

int tcp_connect(const char* ip, int port) {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) return -1;

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &addr.sin_addr);

	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

int tcp_read(int fd, void* buffer, size_t max_len) {
	return read(fd, buffer, max_len);
}

int tcp_write(int fd, const void* buffer, size_t len) {
	return write(fd, buffer, len);
}

// Helper: allocate a buffer and read until CRLF CRLF; returns malloc'd string or NULL
char* tcp_read_request_str(int fd) {
	size_t cap = 8192;
	char* buf = malloc(cap);
	if (!buf) return NULL;
	int n = tcp_read_request(fd, buf, cap);
	if (n <= 0) {
		free(buf);
		return NULL;
	}
	return buf;
}

// Helper: write a NUL-terminated string using tcp_write_all
int tcp_write_string(int fd, const char* s) {
	if (!s) return -1;
	size_t len = strlen(s);
	return tcp_write_all(fd, s, len);
}

void tcp_close(int fd) {
	close(fd);
}