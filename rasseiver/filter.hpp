#ifndef __ILSIMU_RASSEIVER_FILTER_HPP
# define __ILSIMU_RASSEIVER_FILTER_HPP

# include <string>
# include <vector>

# include "circular_buffer.hpp"

using Filter = std::vector<double>;

/**
 * Read filter parameters from a file.  `values' is not cleared.
 *
 * @param file The file to read the configuration from.
 * @param values The vector where the read values are stored.
 */
void filter_read_file(std::string file, Filter &filter);

/**
 * FIR implementation.  The input buffer is convoluted with the filter.
 *
 * This function does not permorm any kind of bound checking when accessing to
 * values in the buffer.  Make sure it is big enough.
 *
 * @param buffer The buffer to filter.
 * @param filter The values of the FIR.
 * @param output The output.
 * @param begin The index of the first element to filter.
 * @param step The decimation factor.  Only one value out of `step` is
 *   filtered.  The result is the same as if the filter was applied to the
 *   whole input, and then decimated, but it's faster to do both at the
 *   same time.
 * @param channels Number of channels in the input.  Useful when the input
 *   is an IQ-demodulated signal.
 */
template<typename T>
size_t filter_buffer(CircularBuffer<T> const &buffer, Filter const &filter,
		     std::vector<T> &output,
		     size_t begin, int step, int channels) {
	size_t i;

	for (i = begin; i < buffer.size(); i += step * channels) {
		T value {};

		for (size_t j = 0; j < filter.size(); ++j) {
			value += buffer[i - (j * channels)] * filter[j];
		}

		output.push_back(value);
	}

	return i;
}

#endif  /* __ILSIMU_RASSEIVER_FILTER_HPP */
