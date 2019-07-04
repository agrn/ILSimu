#include <iostream>

// Signal handling
#include <csignal>
#include <pthread.h>
#include <unistd.h>

#include "config.hpp"
#include "device_airspy.hpp"
#include "device_dummy.hpp"
#include "device_rspduo.hpp"
#include "filter.hpp"

static int wait(unsigned int seconds, sigset_t const &set) {
	int sig;

	alarm(seconds);

	// Wait until we receive a signal
	sigwait(&set, &sig);

	return sig;
}

/**
 * Run the program with the specified device.  Stops when a signal in
 * the sigset is received.
 *
 * TODO: sigset should be part of another function, perhaps.
 *
 * @param device The device to use
 * @param config The configuration of the device.
 * @param filter The filter to be used on the input signal.
 * @param set List of signals to wait for.
 */
template<typename T>
static void run_device(Device<T> &device, ConfigMap const &config,
		       Filter const &filter, sigset_t const &set) {
	Process<T> process {device.buffer_size(), filter,
			config.at("decimation"), device.max_value(),
			config.at("host").get_value(), config.at("port")};
	int sig;

	std::cout << "hello, world" << std::endl;

	// Start receiving data from the device
	Receiver<T> receiver {device, process};

	do {
		sig = wait(1, set);
		// Check every second that the device is still streaming
	} while (sig == SIGALRM && device.is_streaming());

	if (sig == SIGALRM) {
		// If the signal received is an alarm and we are
		// outside of the loop, it means the device stopped
		// streaming.  In this case, throw an exception.
		throw std::runtime_error {"Device stopped streaming"};
	}

	// Stop receiving data from the device and close the process.
	// This is automatically handled by the compiler thanks to
	// RAII.
}

/**
 * Init a sigset_t and use it as a signal mask for every threads.
 *
 * The initialisation step consists of clearing the mask, and adding SIGINT,
 * SIGTERM and SIGALRM to the mask.
 *
 * @param set The signal set to init.
 */
static int setup_sigmask(sigset_t &set) {
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	sigaddset(&set, SIGALRM);

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

	// Read the filter from the disk, if provided
	if (config.count("filter") > 0) {
		filter_read_file(config["filter"].get_value(), filter);
	}

	// Setup signals
	if (setup_sigmask(set)) {
		return EXIT_FAILURE;
	}

	bool retry {false};
	int sig {};

	do {
		try {
			// Check the device type, and call the appropriate function
			if (config["device"] == "airspy") {
				// Determine which airspy to use
				if (config.count("serial_number") > 0) {
					retry = true;

					Airspy airspy {config.at("serial_number"),
							config.at("frequency"),
							config.at("sample_rate"),
							AIRSPY_SAMPLE_INT16_IQ};
					run_device(airspy, config, filter, set);
				} else {
					Airspy airspy {config.at("frequency"),
							config.at("sample_rate"),
							AIRSPY_SAMPLE_INT16_IQ};
					run_device(airspy, config, filter, set);
				}

			} else if (config["device"] == "dummy") {
				DummyDevice dummy {config.at("count")};
				run_device(dummy, config, filter, set);
			} else if (config["device"] == "rspduo") {
				// Determine which airspy to use

					RSPDuo rspduo {config.at("frequency"),
							config.at("sample_rate")};
					run_device(rspduo, config, filter, set);


			}


			 else {
				// Unknown device
				std::cerr << "Unknown device type \""
					  << config["device"].get_value()
					  << "\"" << std::endl;
				return EXIT_FAILURE;
			}
		} catch (std::runtime_error &e) {
			std::cerr << e.what() << std::endl;

			if (retry) {
				sig = wait(1, set);
			} else {
				return EXIT_FAILURE;
			}
		}
	} while (retry && sig == SIGALRM);

	return EXIT_SUCCESS;
}
