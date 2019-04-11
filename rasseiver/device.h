#ifndef __ILSIMU_RASSEIVER_DEVICE_H
# define __ILSIMU_RASSEIVER_DEVICE_H

class Device {
 public:
  Device() = default;
  Device(Device const &) = delete;
  Device &operator=(Device const &) = delete;

  virtual ~Device() {
  };

  virtual void receive() = 0;
};

#endif  /* __ILSIMU_RASSEIVER_DEVICE_H */
