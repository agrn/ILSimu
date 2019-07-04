#ifndef __ILSIMU_RASSEIVER_SENDER_HPP
# define __ILSIMU_RASSEIVER_SENDER_HPP

# include <string>
# include <vector>

# include <sys/socket.h>

/**
 * A RAII wrapper for file descriptors.  In this software, it is used for
 * sockets, but it may be used for other types of files (eg. inotify, etc.)
 */
struct Fd {
	/**
	 * The file descriptor managed by the wrapper.
	 */
	int fd;

	/**
	 * Determines whether the file descriptor is usable or not.
	 */
	bool connected;

	/**
	 * Default constructor.  Initialises `fd' with an invalid value (-1),
	 * and `connected' to false (ie. the file descriptor is unusable).
	 */
	Fd();

	/**
	 * Initialises `fd' with the specified value, and `connected' to true
	 * (ie. the file descriptor is usable).
	 *
	 * @param fd The file descriptor to manage.
	 */
	Fd(int &&fd);

	/**
	 * Calls close().
	 */
	~Fd();

	// No need for copy operators
	Fd(Fd const &) = delete;
	Fd &operator=(Fd const &) = delete;

	// Move semantics
	Fd(Fd &&) = delete; // No need for the move constructor

	/**
	 * Moves the file descriptor contained in `fd' to this structure, copies
	 * its state (ie. `connected'), and sets the parameter to an invalid
	 * state (fd = -1, connected = false).
	 *
	 * @param fd The file descriptor to move.
	 */
	Fd &operator=(Fd &&fd) noexcept;

	/**
	 * If the file descriptor is usable (ie. `connected' is set to true),
	 * closes it and marks it as unusable.
	 */
	void close();
};

/**
 * A class to manage a socket.
 */
class Sender {
public:
	Sender() = delete;

	/**
	 * Connects to the specified server.  If the connection fails, a
	 * std::runtime_error is thrown.
	 *
	 * @param address The address of the server to connect to.
	 * @param port The port of the server.
	 */
	Sender(std::string &&address, uint16_t port);

	/**
	 * Closes the connection.
	 */
	~Sender();

	// No need for those
	Sender(Sender const &) = delete;
	Sender &operator=(Sender const &) = delete;

	/**
	 * Sends a vector to the server.  The socket is closed if the operation
	 * fails.
	 *
	 * A header is sent before the actual data, containing the amount of
	 * data (in bytes, not counting the header size) that will be
	 * transfered, and some flags (eg. whether the data is saturated or
	 * not).  See the Sender::Header structure for more informations.
	 *
	 * The vector is sent after the header.  Both are sent in little-endian.
	 *
	 * @param v The vector to send to the server.
	 * @param saturation Whether the data to send is saturated or not.
	 *   Default to false.
	 * @returns On success, this returns the amount of bytes sent.  On
	 *   error, -1 is returned.
	 */
	template<typename T>
	int send_vector(std::vector<T> const &v, bool saturation=false) {
		int ret {};
		Sender::Header header {v.size() * sizeof(T), saturation};

		if (fd.connected) {
			// Uses Sender::header_size instead of
			// sizeof(Sender::Header), as the structure may be
			// padded with useless bits/bytes.
			ret = send(fd.fd, &header, Sender::header_size, MSG_NOSIGNAL);

			if (ret >= 0)
				ret = send(fd.fd, v.data(), v.size() * sizeof(T), MSG_NOSIGNAL);
		}

		if (ret < 0) {
			fd.close();
		}

		return ret;
	}

	/**
	 * Reconnects to the server.  If this operation fails, the return value
	 * is equal to -1.
	 *
	 * @returns 0 if the connection succeeded, -1 otherwise.
	 */
	int reconnect();

private:
	/**
	 * The header structure of data packets.
	 */
	struct Header {
		/**
		 * The size of the data that will be sent in bytes, not counting
		 * the header itself.
		 */
		const uint64_t size;

		/**
		 * Contains various flags:
		 *   * First bit (LSB): whether the data to be sent is
                 *                      saturated or not.
		 *
		 * The other bits are reserved for future use, and should be set
		 * to 0.
		 */
		const uint8_t flags;
	};

	/**
	 * The size of the header to send.
	 *
	 * Due to data alignment constraints, the Header structure can be padded
	 * and have a size greater than 9 bytes (8 bytes for the size, 1 byte
	 * for the flags) -- on amd64, it is 16 bytes.  Defines the size of the
	 * header to send on the network.
	 *
	 * THIS VALUE SHOULD _ONLY_ BE USED TO COPY OR SEND A HEADER ON THE
	 * NETWORK, _NOT_ FOR MEMORY ALLOCATION PURPOSES.
	 */
	static constexpr size_t header_size = 9;

	const std::string address;
	const uint16_t port;

	Fd fd;
};

#endif  /* __ILSIMU_RASSEIVER_SENDER_HPP */
