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

	virtual ~Device() {
	}

	/**
	 * Receive data from the device.
	 *
	 * @param process The process to apply to the input.
	 */
	virtual void receive(Process<T> &process) = 0;

	/**
	 * Stop receiving data from the device.
	 */
	virtual void stop() = 0;
};

/**
 * An RAII class to start receiving data when constructed, and to stop it when
 * destroyed.
 */
template<class DeviceType, typename T>
class Receiver {
public:
	/**
	 * Start receiving data from the device.
	 *
	 * @param device The device from which data is received.
	 * @param process The process to be applied to a batch of data.
	 */
	Receiver(DeviceType &device, Process<T> &process): device {device} {
		device.receive(process);
	}

	/**
	 * Stop receiving data.
	 */
	~Receiver() {
		device.stop();
	}

private:
	/**
	 * A reference to the device.
	 */
	DeviceType &device;
};

#endif  /* __ILSIMU_RASSEIVER_DEVICE_HPP */
