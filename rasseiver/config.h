#ifndef __ILSIMU_RASSEIVER_CONFIG_H
# define __ILSIMU_RASSEIVER_CONFIG_H

# include <map>
# include <string>

class ConfigValue {
 public:
  ConfigValue() = default;
  ConfigValue(std::string const &value);

  std::string get_value() const;

  operator double() const;
  operator int() const;
  operator unsigned int() const;

  bool operator==(const std::string &s) const;
  bool operator==(const ConfigValue &o) const;

 private:
  std::string value {""};
};

using ConfigMap = std::map<std::string, ConfigValue>;

const ConfigMap config_default = {
  {"device", ConfigValue {"airspy"}},
  {"frequency", ConfigValue {"111100000"}},
  {"sample_rate", ConfigValue {"2500000"}},
  {"sample_type", ConfigValue {"int"}},
};

void config_read_file(std::string file, ConfigMap &config);

#endif  /* __ILSIMU_RASSEIVER_CONFIG_H */
