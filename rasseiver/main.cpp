#include <iostream>

#include "config.h"
#include "device_airspy.h"

/**
 * Run the program with an Airspy device.
 *
 * @param config The configuration of the device
 */
void run_airspy(ConfigMap const &config) {
	Airspy airspy {config.at("frequency"),
			config.at("sample_rate"),
			AIRSPY_SAMPLE_INT16_IQ};

	std::cout << "hello, world" << std::endl;
}

int main(int argc, char **argv) {
	ConfigMap config {config_default};

	// Check program parameters
	if (argc > 2) {
		// Too many parameters
		std::cerr << "Usage: " << argv[0] << " [config file]"
			  << std::endl;
		return EXIT_FAILURE;
	} else if (argc == 2) {
		// If only one parameter was specified, treat it as the path to
		// a config file, and read it.
		std::cout << "Using config file" << std::endl;
		config_read_file(argv[1], config);
	}

	// Check the device type, and call the appropriate function
	if (config["device"] == "airspy") {
		run_airspy(config);
	} else {
		// Unknown device
		std::cerr << "Unknown device type \""
			  << config["device"].get_value()
			  << "\"" << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
