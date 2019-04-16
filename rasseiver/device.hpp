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
	virtual void receive(Process<T> const &process) = 0;
};

#endif  /* __ILSIMU_RASSEIVER_DEVICE_HPP */
