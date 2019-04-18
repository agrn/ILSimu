#include <iostream>

// Signal handling
#include <csignal>
#include <pthread.h>

#include "circular_buffer.hpp"
#include "config.hpp"
#include "device_airspy.hpp"
#include "filter.hpp"

/**
 * Run the program with an Airspy device.  Stops when a signal in the sigset is
 * received.
 *
 * TODO: sigset should be part of another function, perhaps.
 *
 * @param config The configuration of the device.
 * @param set List of signals to wait for.
 * @param filter The filter to be used on the input signal.
 */
void run_airspy(ConfigMap const &config, sigset_t const &set,
		Filter const &filter) {
	Airspy airspy {config.at("frequency"),
			config.at("sample_rate"),
			AIRSPY_SAMPLE_INT16_IQ};
	Process<int16_t> process {65536 * 2, filter, config.at("decimation")};
	int sig;

	std::cout << "hello, world" << std::endl;

	airspy.receive(process);
	sigwait(&set, &sig);
}

/**
 * Init a sigset_t and use it as a signal mask for every threads.
 *
 * The initialisation step consists of clearing the mask, and adding SIGINT and
 * SIGTERM to the mask.
 *
 * @param set The signal set to init.
 */
int setup_sigmask(sigset_t &set) {
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);

	if (pthread_sigmask(SIG_BLOCK, &set, nullptr)) {
		perror("pthread_sigmask()");
		return -1;
	}

	return 0;
}

int main(int argc, char **argv) {
	ConfigMap config {config_default};
	Filter filter;
	sigset_t set;

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

	if (config.count("filter") > 0) {
		filter_read_file(config["filter"].get_value(), filter);
	}

	// Setup signals
	if (setup_sigmask(set)) {
		return EXIT_FAILURE;
	}

	// Check the device type, and call the appropriate function
	if (config["device"] == "airspy") {
		run_airspy(config, set, filter);
	} else {
		// Unknown device
		std::cerr << "Unknown device type \""
			  << config["device"].get_value()
			  << "\"" << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
