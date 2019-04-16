#include "device_airspy.hpp"

#include <iostream>

Airspy::Airspy(unsigned int frequency, unsigned int sample_rate,
	       airspy_sample_type sample_type) {
	airspy_error result;

	result = (airspy_error) airspy_open(&this->device);
	if (result != AIRSPY_SUCCESS) {
		std::cerr << "airspy_open() failed: "
			  << airspy_error_name(result) << std::endl;
		throw std::runtime_error(airspy_error_name(result));
	}

	set_frequency(frequency);
	set_sample_rate(sample_rate);
	set_sample_type(sample_type);
}

Airspy::Airspy(uint32_t serial_num, unsigned int frequency,
	       unsigned int sample_rate, airspy_sample_type sample_type) {
	airspy_error result;

	result = (airspy_error) airspy_open_sn(&this->device, serial_num);
	if (result != AIRSPY_SUCCESS) {
		std::cerr << "airspy_open_sn() failed: "
			  << airspy_error_name(result) << std::endl;
		throw std::runtime_error(airspy_error_name(result));
	}

	set_frequency(frequency);
	set_sample_rate(sample_rate);
	set_sample_type(sample_type);
}

void Airspy::set_frequency(unsigned int frequency) {
	airspy_error result;

	result = (airspy_error) airspy_set_freq(this->device, frequency);
	if (result != AIRSPY_SUCCESS) {
		std::cerr << "airspy_set_freq failed: "
			  << airspy_error_name(result) << std::endl;
		throw std::runtime_error(airspy_error_name(result));
	}
}

void Airspy::set_sample_rate(unsigned int sample_rate) {
	airspy_error result;

	result = (airspy_error) airspy_set_samplerate(this->device,
						      sample_rate);
	if (result != AIRSPY_SUCCESS) {
		std::cerr << "airspy_set_samplerate failed: "
			  << airspy_error_name(result) << std::endl;
		throw std::runtime_error(airspy_error_name(result));
	}
}

void Airspy::set_sample_type(airspy_sample_type sample_type) {
	airspy_error result;

	result = (airspy_error) airspy_set_sample_type(this->device,
						       sample_type);
	if (result != AIRSPY_SUCCESS) {
		std::cerr << "airspy_set_sample_type() failed: "
			  << airspy_error_name(result) << std::endl;
		throw std::runtime_error(airspy_error_name(result));
	}
}

Airspy::~Airspy() {
	if (this->device != nullptr) {
		airspy_stop_rx(device);
		airspy_close(device);
		std::cout << "Closing airspy" << std::endl;
	}
}

static int airspy_callback(airspy_transfer_t *transfer) {
	auto *process = static_cast<Process<int16_t> *> (transfer->ctx);
	int16_t *samples = (int16_t *) transfer->samples;

	process->operator()(samples, transfer->sample_count * 2);

	return 0;
}

void Airspy::receive(Process<int16_t> const &process) {
	airspy_start_rx(device, airspy_callback, (void *) &process);
}
