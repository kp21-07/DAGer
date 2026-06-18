#include "net_utils.h"

#include <cstdint>
#include <sys/socket.h>
#include <arpa/inet.h>

// Sends exactly 'len' bytes. Returns 'len' on success, or <= 0 on error.
ssize_t send_all(int socket, const char* buffer, size_t len)
{
	size_t total_sent = 0;

	while (total_sent < len) {
		ssize_t sent = send(socket, buffer+total_sent, len-total_sent, 0);

		if (sent <= 0) 
			return sent;

		total_sent += sent;
	}

	return total_sent;
}

// Receives exactly 'len' bytes. Returns 'len' on success, or <= 0 on error.
ssize_t recv_all(int socket, char* buffer, size_t len)
{
	size_t total_rcvd = 0;

	while (total_rcvd < len) {
		ssize_t rcvd = recv(socket, buffer+total_rcvd, len-total_rcvd, 0);

		if (rcvd <= 0) 
			return rcvd;

		total_rcvd += rcvd;
	}

	return total_rcvd;
}

// Reads a 32-bit integer from a socket in network byte order and returns host byte order.
bool recv_uint32(int socket, uint32_t& val)
{
	uint32_t network_val;

	if (recv_all(socket, (char *)&network_val, sizeof(network_val)) != sizeof(network_val))
		return false;

	val = ntohl(network_val);
	return true;
}

// Writes a 32-bit integer to a socket in network byte order.
bool send_uint32(int socket, uint32_t val)
{
	uint32_t network_val = htonl(val);

	return send_all(socket, (char *)&network_val, sizeof(network_val)) == sizeof(network_val);
}
