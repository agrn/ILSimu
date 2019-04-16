#ifndef __ILSIMU_RASSEIVER_FILTER_HPP
# define __ILSIMU_RASSEIVER_FILTER_HPP

# include <string>
# include <vector>

using Filter = std::vector<double>;

/**
 * Read filter parameters from a file.  `values' is not cleared.
 *
 * @param file The file to read the configuration from.
 * @param values The vector where the read values are stored.
 */
void filter_read_file(std::string file, Filter &filter);

#endif  /* __ILSIMU_RASSEIVER_FILTER_HPP */
