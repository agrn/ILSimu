#include "config.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>

ConfigValue::ConfigValue(std::string value): value {std::move(value)} {
}

std::string ConfigValue::get_value() const {
	return value;
}

ConfigValue::operator double() const {
	return std::stod(value);
}

ConfigValue::operator int() const {
	return std::stoi(value);
}

ConfigValue::operator unsigned int() const {
	return static_cast<unsigned int> (std::stoi(value));
}

bool ConfigValue::operator==(const std::string &s) const {
	return value == s;
}

bool ConfigValue::operator==(const ConfigValue &o) const {
	return value == o.value;
}

/*
 * ltrim(), rtrim() and trim() were taken from this StackOverflow answer:
 * https://stackoverflow.com/a/217605
 */

/**
 * Delete spaces at the beginning of a string in place.
 *
 * @param s The string to trim.
 */
static inline void ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
				return !std::isspace(ch);
			}));
}

/**
 * Delete spaces at the end of a string in place.
 *
 * @param s The string to trim.
 */
static inline void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
				return !std::isspace(ch);
			}).base(), s.end());
}

/**
 * Delete spaces at the beginning and the end of a string in place.
 *
 * @param s The string to trim.
 */
static inline void trim(std::string &s) {
	ltrim(s);
	rtrim(s);
}

/**
 * Parse a line from a config file, and add the value to `config'.
 *
 * @param line The line to parse.
 * @param config The config map to which parsed parameters are added.
 */
static void parse_line(std::string const &line, ConfigMap &config) {
	std::string key, value;
	bool has_value {false}; // Indicates whether or not the key has been
				// read fully.

	for (auto &it: line) {
		if (it == '#') {
			// `#' are comments until the end of the line, ignores
			// the rest of the line.
			break;
		} else if (it == '=') {
			// The key is over, so all the following characters are
			// part of the key.
			has_value = true;
		} else if (has_value) {
			// If the key has been fully read, add the current
			// char to the value.
			value.append(1, it);
		} else {
			// If the key has not been fully read yet, add the
			// current char to the key.
			key.append(1, it);
		}
	}

	if (has_value) {
		// If a key and a value have been found, remove their trailing
		// whitespaces and add them to the config map.
		trim(key);
		trim(value);

		config[key] = ConfigValue {value};
	}
}

void config_read_file(std::string file, ConfigMap &config) {
	std::ifstream configFile {file};
	std::string line;

	// Opens a file, read it line by line, and parse each line.

	if (!configFile.fail()) {
		while (std::getline(configFile, line)) {
			parse_line(line, config);
		}
	} else {
		std::cerr << "Failed to open file \"" << file << "\""
			  << std::endl;
	}
}
