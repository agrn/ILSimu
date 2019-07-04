#ifndef __ILSIMU_RASSEIVER_DEVICE_DUMMY_HPP
# define __ILSIMU_RASSEIVER_DEVICE_DUMMY_HPP

# include "device.hpp"

# include <atomic>
# include <thread>

/**
 * A class to emulate a device generating IQ samples.
 */
class DummyDevice: public Device<int16_t> {
public:
	// No need for a default constructor.
	DummyDevice() = delete;

	/**
	 * Creates a dummy device to create fake IQ samples.
	 *
	 * @param count The amount of samples to be generated before
	 *   disconnecting.  If it is negative, samples are generated until
	 *   it is stopped with `stop()'.
	 */
	DummyDevice(int count);

	/**
	 * Closes the device.
	 */
	~DummyDevice() override;

	void set_gain(int gain) override {
		(void) gain;
	};

	void receive(Process<int16_t> &process) override;
	void stop() override;

	size_t buffer_size() override;
	int max_value() override;
	bool is_streaming() override;

private:
	std::atomic<bool> running {false};
	std::thread thd;
	const int count;

	static constexpr size_t device_bufsize {65536};
};

#endif  /* __ILSIMU_RASSEIVER_DEVICE_DUMMY_HPP */
