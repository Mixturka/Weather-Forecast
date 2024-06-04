#include "ConfigParser.h"

void Config::Parse(const std::string& path_to_config) {
  Json::Value root;
  Json::CharReaderBuilder builder;
  JSONCPP_STRING errors;
  std::ifstream config(path_to_config);

  if (!Json::parseFromStream(builder, config, &root, &errors)) {
    std::cerr << errors << '\n';
    exit(EXIT_FAILURE);
  }

  update_freq_ = root["dependencies"]["update_frequency"].asInt64();
  forecast_day_count_ = root["dependencies"]["forecast_day_count"].asInt64();
  api_key_ = root["dependencies"]["api-key"].asString();

  for (auto elem : root["cities"]) {
    cities_.push_back(elem.asString());
  }
}

std::vector<std::string> Config::GetCities() { return this->cities_; }

std::string Config::GetApiKey() { return this->api_key_; }

uint64_t Config::GetForecastDayCount() { return this->forecast_day_count_; }

uint64_t Config::GetUpdateFreq() { return this->update_freq_; }
