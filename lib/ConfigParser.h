#pragma once
#include <json/json.h>

#include <fstream>
#include <iostream>
#include <vector>

class Config {
 public:
  void Parse(const std::string &path_to_config);

  std::vector<std::string> GetCities();

  std::string GetApiKey();

  uint64_t GetForecastDayCount();

  uint64_t GetUpdateFreq();

 private:
  uint64_t update_freq_;
  uint64_t forecast_day_count_;
  std::vector<std::string> cities_;
  std::string api_key_;
};
