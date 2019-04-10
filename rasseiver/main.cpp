#include <iostream>

#include "config.h"
#include "device_airspy.h"

void run_airspy(ConfigMap const &config) {
	Airspy airspy {config.at("frequency"),
			config.at("sample_rate"),
			AIRSPY_SAMPLE_INT16_IQ};

	std::cout << "hello, world" << std::endl;
}

int main(int argc, char **argv) {
	ConfigMap config {config_default};

	if (argc > 2) {
		std::cerr << "Usage: " << argv[0] << " [config file]"
			  << std::endl;
		return EXIT_FAILURE;
	} else if (argc == 2) {
		std::cout << "Using config file" << std::endl;
		config_read_file(argv[1], config);
	}

	if (config["device"] == "airspy") {
		run_airspy(config);
	} else {
		std::cerr << "Unknown device type \""
			  << config["device"].get_value()
			  << "\"" << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
