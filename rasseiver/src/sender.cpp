#include <stdexcept>

#include <cerrno>
#include <unistd.h>
#include <cstring>

#include <arpa/inet.h>

#include "sender.hpp"

Fd::Fd(): fd {-1}, connected {false} {
}

Fd::Fd(int &&fd): fd {fd}, connected {true} {
}

Fd::~Fd() {
	close();
}

Fd &Fd::operator=(Fd &&fd) noexcept {
	close();

	Fd::fd = fd.fd;
	Fd::connected = fd.connected;

	fd.fd = -1;
	fd.connected = false;

	return *this;
}

void Fd::close() {
	if (connected) {
		::close(fd);
		connected = false;
	}
}

Sender::Sender(std::string const &address, uint16_t port):
	address {address}, port {port} {
	if (reconnect() != 0) {
		throw std::runtime_error(
			"Failed to connect to " + address + ":" + std::to_string(port) + ": " +
			std::strerror(errno));
	}
}

Sender::~Sender() {
	fd.close();
}

int Sender::reconnect() {
	fd.close();

	Fd newFd {socket(AF_INET, SOCK_STREAM, 0)};
	struct sockaddr_in server_addr;

	bzero(&server_addr, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	if (inet_pton(AF_INET, address.c_str(), &server_addr.sin_addr) != 1) {
		return -1;
	}

	if (connect(newFd.fd, (struct sockaddr *) &server_addr,
		    sizeof(struct sockaddr_in))) {
		return -1;
	}

	fd = std::move(newFd);

	return 0;
}
