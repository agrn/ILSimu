#ifndef DEVICE_RSPDUO_HPP
#define DEVICE_RSPDUO_HPP

#include "sdrplay_api.h"

#include "device.hpp"


/**
 * A class to encapsulate an RSPDuo device.  It uses the sdrplay
 * api.
 *
 * This class offers some advantages.  Mainly, this is an RAII class; when
 * allocated on the stack or with an unique_ptr, the device will
 * be automatically closed.
 */
class RSPDuo: public Device<int16_t> {
public:
	// No need for a default constructor.
	RSPDuo() = delete;

	/**
	 * Opens an Airspy device with a specified frequency, sample rate,
	 * and sample type.
	 *
	 * @param frequency The frequency of the signal to sample.
	 * @param sample_rate Amount of samples the RSPDuo should take in a second.
	 */
	RSPDuo(unsigned int frequency=111100000,
	       unsigned int sample_rate=2500000);

	/**
	 * Opens an Airspy device with a specific serial number, a specified
	 * frequency, sample rate, and sample type.
	 *
	 * @param serial_num The serial number of the Airspy to open.
	 * @param frequency The frequency of the signal to sample.
	 * @param sample_rate Amount of samples the Airspy should take in a second.
	 * @param sample_type Type of samples the Airspy should take.
	 */

	/**
	 * Closes the RSPDuo.
	 */
	~RSPDuo() override;


	void set_gain(int gain) override;
	bool is_streaming();

	void receive(Process<int16_t> &process) override;
	void stop() override;

	size_t buffer_size() override;
	int max_value() override;

private:
	void init_RSPDuo(unsigned int frequency, unsigned int sample_rate);
	sdrplay_api_DeviceT devs[6];unsigned int ndev;
	int i;
	float ver= 0.0;
	sdrplay_api_ErrT err;
	sdrplay_api_DeviceParamsT *deviceParams = NULL;
	sdrplay_api_CallbackFnsT cbFns;
	sdrplay_api_RxChannelParamsT *chParams;

	int reqTuner = 0;
	int master_slave = 0;
	unsigned int chosenIdx = 0;

	sdrplay_api_DeviceT *device {nullptr};
	sdrplay_api_DeviceT *chosenDevice = NULL;
};

#endif // DEVICE_RSPDUO_HPP
