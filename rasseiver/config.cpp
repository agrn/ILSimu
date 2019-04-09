#include <fstream>
#include <map>
#include <sstream>

#include "config.h"

ConfigValue::ConfigValue(): value {""} {
}

ConfigValue::ConfigValue(std::string value): value {value} {
}

ConfigValue::operator std::string() const {
	return this->value;
}

ConfigValue::operator double() const {
	return std::stod(value);
}

ConfigValue::operator int() const {
	return std::stoi(value);
}
