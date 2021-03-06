#ifndef __ILSIMU_RASSEIVER_CONFIG_HPP
# define __ILSIMU_RASSEIVER_CONFIG_HPP

# include <map>
# include <string>

/**
 * Stores a value from the config file. It can then be converted to a double
 * precision floating number, an integer or an unsigned integer.
 */
class ConfigValue {
public:
	/**
	 * Creates a ConfigValue with an empty string.
	 */
	ConfigValue() = default;

	/**
	 * Creates a ConfigValue with the specified value.
	 *
	 * @param value The value to set.
	 */
	ConfigValue(std::string value);

	std::string get_value() const;

	operator double() const;
	operator int() const;
	operator unsigned int() const;
	operator uint64_t() const;

	bool operator==(std::string const &s) const;
	bool operator==(ConfigValue const &o) const;

private:
	std::string value {""};
};

using ConfigMap = std::map<std::string, ConfigValue>;

/**
 * The default configuration. Set to use an Airspy device at a frequency of
 * 111.1 MHz, 2.5 MSPS, sample integers, decimate at a factor of 60, and connect
 * to the server at 127.0.0.1:10001.  If the device used is the dummydevice, it
 * will run until it is interrupted by the user.
 */
const ConfigMap config_default {
	{"device", ConfigValue {"airspy"}},
	{"frequency", ConfigValue {"111100000"}}, // 111.1 MHz
	{"sample_rate", ConfigValue {"2500000"}}, // 2.5 MSPS
	{"sample_type", ConfigValue {"int"}},
	{"decimation", ConfigValue {"60"}},
	{"host", ConfigValue {"127.0.0.1"}},
	{"port", ConfigValue {"10001"}},
	{"count", ConfigValue {"-1"}}, // For dummydevice
};

/**
 * Parse the specified config file. `config' is not cleared before reading the
 * file, but all existing values are replaced.
 *
 * @param file The file to read the configuration from.
 * @param config The map to which parsed parameters are added.
 */
void config_read_file(std::string const &file, ConfigMap &config);

#endif  /* __ILSIMU_RASSEIVER_CONFIG_HPP */
