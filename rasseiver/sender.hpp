#ifndef __ILSIMU_RASSEIVER_SENDER_HPP
# define __ILSIMU_RASSEIVER_SENDER_HPP

# include <string>
# include <vector>

# include <sys/types.h>
# include <sys/socket.h>

class Sender {
public:
	Sender() = delete;

	Sender(std::string const &address, uint16_t port);
	~Sender();

	Sender(Sender const &) = delete;
	Sender &operator=(Sender const &) = delete;

	template<typename T>
	void send_vector(std::vector<T> const &v) {
		// TODO: handle EPIPE & reconnect.
		send(fd, v.data(), v.size() * sizeof(T), MSG_NOSIGNAL);
	}

private:
	int reconnect();

	const std::string address;
	const uint16_t port;

	bool connected;
	int fd;
};

#endif  /* __ILSIMU_RASSEIVER_SENDER_HPP */
