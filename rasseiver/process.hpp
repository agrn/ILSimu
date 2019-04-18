#ifndef __ILSIMU_RASSEIVER_PROCESS_HPP
# define __ILSIMU_RASSEIVER_PROCESS_HPP

# include <array>
# include <vector>


# include "circular_buffer.hpp"
# include "filter.hpp"

template<typename T>
class Process {
public:
	Process() = delete;

	Process(size_t bufsize, Filter filter, int step):
		buf {bufsize}, output (bufsize), filter {std::move(filter)},
		pos {0}, step {step} {
	}

	Process(size_t bufsize, Filter &&filter, int step):
		buf {bufsize}, output (bufsize), filter {std::move(filter)},
		pos {0}, step {step} {
	}

	Process(Process const &) = delete;
	Process &operator=(Process const &) = delete;

	void apply(T *input, size_t count) {
		output.clear();

		buf.switch_buffer(input, count);


		pos = filter_buffer(buf, filter, output, pos, step)
			% buf.size();
	}

private:
	CircularBuffer<T> buf;
	std::vector<T> output;
	Filter filter;

	size_t pos;
	int step;
};

#endif  /* __ILSIMU_RASSEIVER_PROCESS_HPP */
