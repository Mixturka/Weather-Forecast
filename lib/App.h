#pragma once
#include <json/json.h>
#include <termios.h>

#include <chrono>
#include <fstream>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <iostream>
#include <thread>

#include "ForecastManager.h"
#include "ftxui/component/loop.hpp"

using namespace ftxui;

class App {
 public:
  void Run(const std::string& config_path);
  Element BuildUI(Config& config, std::string& city_);

 private:
  uint64_t day_count;

  void InputThread(bool& is_running, int& day_count, Screen& screen,
                   Config& config, Element& doc,
                   std::vector<std::string>& cities, int& y, int& current_city);

  Element GetCloud(const uint8_t weather_code);

  Element FormSummary(Json::Value value);

  std::string RoundFloat(const std::string& temperature);

  std::string GetDescOfWWOCodes(const uint8_t& weather_code);

  std::string GetWindDirection(const std::string& degrees);
};
