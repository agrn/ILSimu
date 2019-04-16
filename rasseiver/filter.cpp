#include <fstream>
#include <iostream>
#include <vector>

#include "filter.hpp"

void filter_read_file(std::string file, Filter &filter) {
	std::ifstream valueFile {file};
	std::string line;

	if (!valueFile.fail()) {
		while (std::getline(valueFile, line)) {
			try {
				double value = std::stod(line);
				filter.push_back(value);
			} catch (std::invalid_argument e) {
				// Not a number, ignoring
			}
		}
	} else {
		std::cerr << "Failed to open file \"" << file << "\""
			  << std::endl;
	}
}
