#ifndef __ILSIMU_RASSEIVER_DEVICE_AIRSPY_HPP
# define __ILSIMU_RASSEIVER_DEVICE_AIRSPY_HPP

# include "device.hpp"

# include <airspy.h>

/**
 * A class to encapsulate an Airspy device.  It uses the libairspy
 * library.
 *
 * This class offers some advantages over the direct use of the
 * airspy_device struct.  Mainly, this is an RAII class; when
 * allocated on the stack or with an unique_ptr, the device will
 * be automatically closed.
 */
class Airspy: public Device<int16_t> {
public:
	// No need for a default constructor.
	Airspy() = delete;

	/**
	 * Opens an Airspy device with a specified frequency, sample rate,
	 * and sample type.
	 *
	 * @param frequency The frequency of the signal to sample.
	 * @param sample_rate Amount of samples the Airspy should take in a second.
	 * @param sample_type Type of samples the Airspy should take.
	 */
	Airspy(unsigned int frequency=111100000,
	       unsigned int sample_rate=2500000,
	       airspy_sample_type sample_type=AIRSPY_SAMPLE_INT16_IQ);

	/**
	 * Opens an Airspy device with a specific serial number, a specified
	 * frequency, sample rate, and sample type.
	 *
	 * @param serial_num The serial number of the Airspy to open.
	 * @param frequency The frequency of the signal to sample.
	 * @param sample_rate Amount of samples the Airspy should take in a second.
	 * @param sample_type Type of samples the Airspy should take.
	 */
	Airspy(uint32_t serial_num,
	       unsigned int frequency=111100000,
	       unsigned int sample_rate=2500000,
	       airspy_sample_type sample_type=AIRSPY_SAMPLE_INT16_IQ);

	/**
	 * Closes the Airspy.
	 */
	~Airspy() override;

	bool is_streaming();

	void set_frequency(unsigned int frequency);
	void set_sample_rate(unsigned int sample_rate);
	void set_sample_type(airspy_sample_type sample_type);

	void receive(Process<int16_t> &process) override;
	void stop() override;

	size_t buffer_size() override;
	int max_value() override;

private:
	airspy_device *device {nullptr};
};

#endif  /* __ILSIMU_RASSEIVER_DEVICE_AIRSPY_HPP */
