#ifndef __ILSIMU_RASSEIVER_DEVICE_AIRSPY_H
# define __ILSIMU_RASSEIVER_DEVICE_AIRSPY_H

# include "device.h"

# include <airspy.h>

class Airspy: public Device {
 public:
  Airspy(unsigned int frequency=111100000,
	 unsigned int sample_rate=2500000,
	 airspy_sample_type sample_type=AIRSPY_SAMPLE_INT16_IQ);
  Airspy(uint32_t serial_num,
	 unsigned int frequency=111100000,
	 unsigned int sample_rate=2500000,
	 airspy_sample_type sample_type=AIRSPY_SAMPLE_INT16_IQ);
  ~Airspy();

  virtual void receive();

  void set_frequency(unsigned int frequency);
  void set_sample_rate(unsigned int sample_rate);
  void set_sample_type(airspy_sample_type sample_type);

 private:
  airspy_device *device {nullptr};
};

#endif  /* __ILSIMU_RASSEIVER_DEVICE_AIRSPY_H */
