#include <fstream>
#include <iostream>
#include <vector>

void filter_read_file(std::string file, std::vector<double> &values) {
	std::ifstream valueFile {file};
	std::string line;

	if (!valueFile.fail()) {
		while (std::getline(valueFile, line)) {
			try {
				double value = std::stod(line);
				values.push_back(value);
			} catch (std::invalid_argument e) {
				// Not a number, ignoring
			}
		}
	} else {
		std::cerr << "Failed to open file \"" << file << "\""
			  << std::endl;
	}
}
