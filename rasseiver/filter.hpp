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
 */
template<typename T>
size_t filter_buffer(CircularBuffer<T> const &buffer, Filter const &filter,
		     std::vector<T> &output,
		     size_t begin, int step) {
	size_t i;

	for (i = begin; i < buffer.size(); i += step * 2) {
		T valueI {}, valueQ {};

		for (size_t j = 0; j < filter.size(); ++j) {
			valueI += buffer[i - (j * 2)] * filter[j];
			valueQ += buffer[i + 1 - (j * 2)] * filter[j];
		}

		output.push_back(valueI);
		output.push_back(valueQ);
	}

	return i;
}

#endif  /* __ILSIMU_RASSEIVER_FILTER_HPP */
