#include "device_airspy.hpp"

#include <iostream>

Airspy::Airspy(unsigned int frequency, unsigned int sample_rate,
	       airspy_sample_type sample_type) {
	airspy_error result;

	result = (airspy_error) airspy_open(&this->device);
	if (result != AIRSPY_SUCCESS) {
		std::cerr << "airspy_open() failed: "
			  << airspy_error_name(result) << std::endl;
		throw std::runtime_error {airspy_error_name(result)};
	}

	init_airspy(frequency, sample_rate, sample_type);
}

Airspy::Airspy(uint32_t serial_num, unsigned int frequency,
	       unsigned int sample_rate, airspy_sample_type sample_type) {
	airspy_error result;

	result = (airspy_error) airspy_open_sn(&this->device, serial_num);
	if (result != AIRSPY_SUCCESS) {
		std::cerr << "airspy_open_sn() failed: "
			  << airspy_error_name(result) << std::endl;
		throw std::runtime_error {airspy_error_name(result)};
	}

	init_airspy(frequency, sample_rate, sample_type);
}

void Airspy::init_airspy(unsigned int frequency, unsigned int sample_rate,
			 airspy_sample_type sample_type) {
	airspy_error result;

	set_frequency(frequency);
	set_sample_rate(sample_rate);
	set_sample_type(sample_type);

	result = (airspy_error) airspy_set_vga_gain(device, 5);
	if (result == AIRSPY_SUCCESS)
		result = (airspy_error) airspy_set_mixer_gain(device, 5);
	if (result == AIRSPY_SUCCESS)
		result = (airspy_error) airspy_set_lna_gain(device, 1);

	if (result != AIRSPY_SUCCESS) {
		std::cerr << "Failed to set Airspy gain: "
			  << airspy_error_name(result) << std::endl;
		throw std::runtime_error {airspy_error_name(result)};
	}
}

bool Airspy::is_streaming() {
	return airspy_is_streaming(device);
}

void Airspy::set_frequency(unsigned int frequency) {
	airspy_error result;

	result = (airspy_error) airspy_set_freq(this->device, frequency);
	if (result != AIRSPY_SUCCESS) {
		std::cerr << "airspy_set_freq failed: "
			  << airspy_error_name(result) << std::endl;
		throw std::runtime_error {airspy_error_name(result)};
	}
}

void Airspy::set_sample_rate(unsigned int sample_rate) {
	airspy_error result;

	result = (airspy_error) airspy_set_samplerate(this->device,
						      sample_rate);
	if (result != AIRSPY_SUCCESS) {
		std::cerr << "airspy_set_samplerate failed: "
			  << airspy_error_name(result) << std::endl;
		throw std::runtime_error {airspy_error_name(result)};
	}
}

void Airspy::set_sample_type(airspy_sample_type sample_type) {
	airspy_error result;

	result = (airspy_error) airspy_set_sample_type(this->device,
						       sample_type);
	if (result != AIRSPY_SUCCESS) {
		std::cerr << "airspy_set_sample_type() failed: "
			  << airspy_error_name(result) << std::endl;
		throw std::runtime_error {airspy_error_name(result)};
	}
}

Airspy::~Airspy() {
	if (this->device != nullptr) {
		airspy_close(device);
		std::cout << "Closing airspy" << std::endl;
	}
}

/**
 * A callback to wrap the real process to apply.
 *
 * This functions checks if the Airspy is still synced with its clock, and calls
 * the process provided.
 *
 * To check if the Airspy is still synced, the register 0 is dumped.  If the 4th
 * bit is set, the device is still synced.  If not, the device is out of sync,
 * and a warning is printed to stderr.

 * Then, it converts the context parameter to a Process, and calls its method
 * apply().  This call should be inlined when the program is optimised.
 *
 * This function delegates the processing instead of doing itself, because if we
 * wanted to handle another device kind, we would have to duplicate the process
 * in the other callback.  Moving the process makes it agnostic of the
 * underlying device and reusable.  Some parameters are needed (such as the
 * filter used), so it's a class and not a freestanding function.
 *
 * @param transfer The data received from the Airspy.
 */
static int airspy_callback(airspy_transfer_t *transfer) {
	// Dump register and check if the airspy is still synced.
	uint8_t value;
	int result {airspy_si5351c_read(transfer->device, 0, &value)};
	if (result != AIRSPY_SUCCESS) {
		std::cerr << "Error: could not dump register." << std::endl;
		std::cerr << airspy_error_name((airspy_error) result)
			  << std::endl;
	} else if (value & 0x10) {
		std::cerr << "Warning: Airspy out of sync." << std::endl;
	}

	if (transfer->dropped_samples > 0) {
		std::cerr << "Dropped samples" << std::endl;
	}

	// Processing input buffer
	// Get back the process and the samples.
	auto *process {static_cast<Process<int16_t> *> (transfer->ctx)};
	int16_t *samples {(int16_t *) transfer->samples};

	process->apply(samples, transfer->sample_count * 2);

	return 0;
}

void Airspy::receive(Process<int16_t> &process) {
	auto result {airspy_start_rx(device, airspy_callback,
				     (void *) &process)};

	if (result != AIRSPY_SUCCESS) {
		throw std::runtime_error {
			std::string {"airspy_start_rx() failed: "} +
			airspy_error_name((airspy_error) result)};
	}
}

void Airspy::stop() {
	airspy_stop_rx(device);
}

size_t Airspy::buffer_size() {
	// 262144 is from libairspy.  This is the size of the buffer in bytes.
	// Divided by 2 for both IQ channels, then by the size of an int16_t
	// (should be 2).  The final result should be equal to 65536.
	return 262144 / 2 / sizeof(int16_t);
}

int Airspy::max_value() {
	// 2^12
	return 4069;
}
