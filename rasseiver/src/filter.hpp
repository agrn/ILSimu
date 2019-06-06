#ifndef __ILSIMU_RASSEIVER_FILTER_HPP
# define __ILSIMU_RASSEIVER_FILTER_HPP

# include <string>
# include <vector>

# include <cmath>

# include "circular_buffer.hpp"

using Filter = std::vector<double>;

/**
 * Read filter parameters from a file.  `values' is not cleared.
 *
 * @param file The file to read the configuration from.
 * @param values The vector where the read values are stored.
 */
void filter_read_file(std::string const &file, Filter &filter);

/**
 * FIR implementation.  The input buffer is convoluted with the filter.  The
 * input buffer is assumed to contain interleaved I and Q samples.
 *
 * This function does not permorm any kind of bound checking when accessing to
 * values in the buffer.  Make sure it is big enough.
 *
 * Saturation occurs when the modulus of an IQ sample is higher or equal to the
 * threshold.  Although it is not the best way to calculate it, it's relatively
 * fast and simple.
 *
 * @param buffer The buffer to filter.
 * @param filter The values of the FIR.
 * @param output The output.
 * @param begin The index of the first element to filter.  It is used by the
 *   loop as an index and is incremented in `step * 2' incrementns.  When the
 *   function returns, its value will be between `buffer.size()' and
 *   `buffer.size() + step * 2'.
 * @param step The decimation factor.  Only one value out of `step` is
 *   filtered.  The result is the same as if the filter was applied to the
 *   whole input, and then decimated, but it's faster to do both at the
 *   same time.
 * @return True if a saturation occurs, otherwise false.
 */
template<typename T>
bool filter_buffer(CircularBuffer<T> const &buffer, Filter const &filter,
		   std::vector<T> &output, size_t &begin, int step,
		   int threshold) {
	size_t &i {begin};
	bool saturation {false};
	std::vector<T> const &previous {buffer.get_previous()},
		&current {buffer.get_current()};

	for (; i < buffer.size(); i += step * 2) {
		double valueI {}, valueQ {};
		int j {(int) filter.size() - 1}, k {(int) (i - (j * 2))};

		if (k < 0) {
			k += previous.size();

			while (j >= 0 && k < (int) previous.size()) {
				valueI += previous[k++] * filter[j];
				valueQ += previous[k++] * filter[j--];
			}

			k = 0;
		}

		while (j >= 0) {
			valueI += current[k++] * filter[j];
			valueQ += current[k++] * filter[j--];
		}

		output.push_back(std::round(valueI));
		output.push_back(std::round(valueQ));

		if (std::sqrt(valueI * valueI + valueQ * valueQ) >= threshold) {
			saturation = true;
		}
	}

	return saturation;
}

#endif  /* __ILSIMU_RASSEIVER_FILTER_HPP */
