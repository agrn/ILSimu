#ifndef __ILSIMU_RASSEIVER_FILTER_HPP
# define __ILSIMU_RASSEIVER_FILTER_HPP

# include <string>
# include <vector>

/**
 * Read filter parameters from a file.  `values' is not cleared.
 *
 * @param file The file to read the configuration from.
 * @param values The vector where the read values are stored.
 */
void filter_read_file(std::string file, std::vector<double> &values);

#endif  /* __ILSIMU_RASSEIVER_FILTER_HPP */
