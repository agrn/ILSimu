#include "config.h"

#include <algorithm>
#include <fstream>
#include <iostream>

ConfigValue::ConfigValue(std::string const &value): value {value} {
}

std::string ConfigValue::get_value() const {
	return this->value;
}

ConfigValue::operator double() const {
	return std::stod(value);
}

ConfigValue::operator int() const {
	return std::stoi(value);
}

ConfigValue::operator unsigned int() const {
	return (unsigned int) std::stoi(value);
}

bool ConfigValue::operator==(const std::string &s) const {
	return this->value == s;
}

bool ConfigValue::operator==(const ConfigValue &o) const {
	return this->value == o.value;
}

static inline void ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
				return !std::isspace(ch);
			}));
}

static inline void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
				return !std::isspace(ch);
			}).base(), s.end());
}

static inline void trim(std::string &s) {
	ltrim(s);
	rtrim(s);
}

static void parse_line(std::string const &line, ConfigMap &config) {
	std::string key, value;
	bool has_value {false};

	for (auto it = line.begin(); it != line.end(); ++it) {
		if (*it == '#') {
			break;
		} else if (*it == '=') {
			has_value = true;
		} else if (*it == '\\') {
			++it;
		} else if (has_value) {
			value.append(1, *it);
		} else {
			key.append(1, *it);
		}
	}

	if (has_value) {
		trim(key);
		trim(value);

		config[key] = ConfigValue {value};
	}
}

void config_read_file(std::string file, ConfigMap &config) {
	std::ifstream configFile {file};
	std::string line;

	if (!configFile.fail()) {
		while (std::getline(configFile, line)) {
			parse_line(line, config);
		}
	} else {
		std::cerr << "Failed to open file \"" << file << "\""
			  << std::endl;
	}
}
