#ifndef __ILSIMU_RASSEIVER_PROCESS_HPP
# define __ILSIMU_RASSEIVER_PROCESS_HPP

# include <array>
# include <vector>

# include <numeric>

# include "circular_buffer.hpp"
# include "filter.hpp"

template<typename T>
class Process {
public:
	Process() = delete;

	Process(size_t bufsize, Filter const &filter, int step):
		buf {bufsize}, output (bufsize), filter {filter}, step {step} {
		std::iota(std::begin(pos), std::end(pos), 0);
	}

	Process(size_t bufsize, Filter &&filter, int step):
		buf {bufsize}, output (bufsize), filter {std::move(filter)},
		step {step} {
		std::iota(std::begin(pos), std::end(pos), 0);
	}

	Process(Process const &) = delete;
	Process &operator=(Process const &) = delete;

	void operator()(T *input, size_t count) {
		output.clear();

		buf.switch_buffer(input, count);

		pos[0] = filter_buffer(buf, filter, output, pos[0], 60, 2)
			% buf.size();

		pos[1] = filter_buffer(buf, filter, output, pos[1], 60, 2)
			% buf.size();
	}

private:
	CircularBuffer<T> buf;
	std::array<size_t, 2> pos;
	std::vector<T> output;
	Filter filter;

	int step;
};

#endif  /* __ILSIMU_RASSEIVER_PROCESS_HPP */
