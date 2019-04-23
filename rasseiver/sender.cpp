#include <iostream>

#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <strings.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "sender.hpp"

static inline void throw_error(std::string &&message) {
	throw std::runtime_error(message + ": " + std::strerror(errno));
}

Sender::Sender(std::string const &address, uint16_t port):
	address {address}, port {port}, connected {false}, fd {reconnect()} {
	connected = true;
}

Sender::~Sender() {
	if (connected) {
		close(fd);
		std::cout << "a+" << std::endl;
	}
}

int Sender::reconnect() {
	if (connected) {
		close(fd);
		connected = false;
	}

	int fd {socket(AF_INET, SOCK_STREAM, 0)};
	struct sockaddr_in server_addr;

	bzero(&server_addr, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	if (inet_pton(AF_INET, address.c_str(), &server_addr.sin_addr) != 1) {
		throw_error("inet_pton()");
	}

	if (connect(fd, (struct sockaddr *) &server_addr,
		    sizeof(struct sockaddr_in))) {
		close(fd);
		throw_error("connect()");
	}

	return fd;
}
