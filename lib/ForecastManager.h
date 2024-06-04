#pragma once
#include <cpr/cpr.h>
#include <json/json.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "ConfigParser.h"

class ForecastManager {
 public:
  void WriteForecast(Config& config);

  std::vector<std::string> GetCities();

 private:
  Json::Value root_;

  void ParseOpenMeteoResponse(const cpr::Response& response);
};
