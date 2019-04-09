#ifndef __ILSIMU_RASSEIVER_CONFIG_H
# define __ILSIMU_RASSEIVER_CONFIG_H

# include <string>

class ConfigValue {
 public:
	ConfigValue();
	ConfigValue(std::string value);

	operator std::string() const;
	operator double() const;
	operator int() const;

 private:
	std::string value;
};

#endif  /* __ILSIMU_RASSEIVER_CONFIG_H */
