#ifndef __ILSIMU_RASSEIVER_DEVICE_HPP
# define __ILSIMU_RASSEIVER_DEVICE_HPP

/**
 * An abstract device class.
 */
class Device {
public:
	/**
	 * A default constructor for children classes.
	 */
	Device() = default;

	Device(Device const &) = delete;
	Device &operator=(Device const &) = delete;

	virtual ~Device() {
	};

	/**
	 * Receive data from the device.
	 */
	virtual void receive() = 0;
};

#endif  /* __ILSIMU_RASSEIVER_DEVICE_HPP */
