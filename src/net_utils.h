#ifndef NET_UTILS_H
#define NET_UTILS_H

#include <stddef.h>
#include <sys/types.h>
#include <stdint.h>

ssize_t send_all(int socket, const char* buffer, size_t len);

ssize_t recv_all(int socket, char* buffer, size_t len);

bool recv_uint32(int socket, uint32_t& val);

bool send_uint32(int socket, uint32_t val);

#endif

