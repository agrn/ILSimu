#ifndef __ILSIMU_RASSEIVER_DEVICE_HPP
# define __ILSIMU_RASSEIVER_DEVICE_HPP

# include "process.hpp"

/**
 * An abstract device class.
 */
template<typename T>
class Device {
public:
	/**
	 * A default constructor for children classes.
	 */
	Device() = default;

	Device(Device const &) = delete;
	Device &operator=(Device const &) = delete;

	virtual ~Device() = default;

	/**
	 * Receive data from the device.
	 *
	 * @param process The process to apply to the input.
	 */
	virtual void receive(Process<T> &process) = 0;

	/**
	 * Set gain
	 */
	virtual void set_gain(int gain) = 0;

	/**
	 * Stop receiving data from the device.
	 */
	virtual void stop() = 0;

	/**
	 * Returns the size of the internal buffer of the device.
	 */
	virtual size_t buffer_size() = 0;

	/**
	 * Returns the max value that the device can sample.
	 */
	virtual int max_value() = 0;

	virtual bool is_streaming() = 0;
};

/**
 * An RAII class to start receiving data when constructed, and to stop it when
 * destroyed.
 */
template<typename T>
class Receiver {
public:
	/**
	 * Start receiving data from the device.
	 *
	 * @param device The device from which data is received.
	 * @param process The process to be applied to a batch of data.
	 */
	Receiver(Device<T> &device, Process<T> &process): device {device} {
		device.receive(process);
	}

	/**
	 * Stop receiving data.
	 */
	~Receiver() {
		device.stop();
	}

	Receiver(Receiver const &) = delete;
	Receiver &operator=(Receiver const &) = delete;

private:
	/**
	 * A reference to the device.
	 */
	Device<T> &device;
};

#endif  /* __ILSIMU_RASSEIVER_DEVICE_HPP */
