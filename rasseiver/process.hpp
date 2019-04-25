#ifndef __ILSIMU_RASSEIVER_PROCESS_HPP
# define __ILSIMU_RASSEIVER_PROCESS_HPP

# include <array>
# include <vector>

# include "circular_buffer.hpp"
# include "filter.hpp"
# include "sender.hpp"

/**
 * Defines a process to apply to an input buffer.
 *
 * This is a class and not a raw function, because it needs to carry multiple
 * parameters, such as the filter.  It is agnostic of the underlying device, and
 * should be easily reusable.
 */
template<typename T>
class Process {
public:
	// No need for a default constructor
	Process() = delete;

	/**
	 * Creates a new process with a specified size, filter, and decimation
	 * factor.
	 *
	 * @param bufsize The size of the input buffer to create.
	 * @param filter The filter to create.  This version copies the filter.
	 * @param step The decimation factor.
	 * @param threshold The max value that the device associated with this
	 *   process can sample.  Multiplied by 92%, and is used to detect
	 *   saturation.
	 */
	Process(size_t bufsize, Filter filter, int step, int threshold,
		std::string &&host, unsigned int port):
		buf {bufsize}, output (bufsize), filter {std::move(filter)},
		pos {0}, step {step}, threshold {(int) (threshold * 0.92)},
		sender {host, (uint16_t) port} {
	}

	/**
	 * Creates a new process with a specified size, filter, and decimation
	 * factor.
	 *
	 * @param bufsize The size of the input buffer to create.
	 * @param filter The filter to create.  This version moves the buffer.
	 * @param step The decimation factor.
	 * @param threshold The max value that the device associated with this
	 *   process can sample.  Multiplied by 92%, and is used to detect
	 *   saturation.
	 */
	Process(size_t bufsize, Filter &&filter, int step, int threshold,
		std::string &&host, unsigned int port):
		buf {bufsize}, output (bufsize), filter {std::move(filter)},
		pos {0}, step {step}, threshold {(int) (threshold * 0.92)},
		sender {host, (uint16_t) port} {
	}

	// No need for these
	Process(Process const &) = delete;
	Process &operator=(Process const &) = delete;

	/**
	 * Apply the process to the input buffer.
	 *
	 * @param input The raw input data from the buffer.  It is expected to
	 *   have interleaved I and Q values.
	 * @param count The size of the buffer.
	 */
	void apply(T *input, size_t count) {
		output.clear();

		buf.switch_buffer(input, count);

		bool saturation {filter_buffer(buf, filter, output, pos, step,
					       threshold)};
		pos %= buf.size();

		(void) saturation;

		if (sender.send_vector<T>(output) <= 0) {
			sender.reconnect();
		}
	}

private:
	CircularBuffer<T> buf;
	std::vector<T> output;
	Filter filter;

	size_t pos;
	int step;
	int threshold;

	Sender sender;
};

#endif  /* __ILSIMU_RASSEIVER_PROCESS_HPP */
