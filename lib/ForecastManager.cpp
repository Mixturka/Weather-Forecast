#include "ForecastManager.h"

#include <cstdio>

std::vector<std::string> ForecastManager::GetCities() {
  Json::Value root;
  Json::Reader reader;
  Json::CharReaderBuilder builder;
  JSONCPP_STRING errors;
  std::ifstream data("forecast.json");

  if (!Json::parseFromStream(builder, data, &root, &errors)) {
    std::cerr << errors << '\n';
    exit(EXIT_FAILURE);
  }
  std::vector<std::string> cities;
  for (std::string name : root.getMemberNames()) {
    cities.push_back(name);
  }

  return cities;
}

void ForecastManager::ParseOpenMeteoResponse(const cpr::Response& response) {
  Json::Value root;
  Json::Reader reader;
}

void ForecastManager::WriteForecast(Config& config) {
  std::ofstream output("forecast.json");

  Json::Value root;
  Json::Reader reader;
  Json::Value result;
  for (std::string city : config.GetCities()) {
    cpr::Response location_response =
        cpr::Get(cpr::Url{"https://api.api-ninjas.com/v1/city?name=" + city},
                 cpr::Header{{"X-Api-Key", config.GetApiKey()}});

    if (location_response.status_code != 200) {
      std::cout << "Error retrieving location data for city: " << city
                << std::endl;
      exit(1);
    }

    std::string location_json = std::string(location_response.text);

    if (!reader.parse(location_json, root)) {
      std::cout << "Error parsing location JSON for city: " << city
                << std::endl;
      continue;
    }

    float latitude = root[0]["latitude"].asFloat();
    float longitude = root[0]["longitude"].asFloat();
    // std::cout << latitude << ' ' << longitude << ' ' << city;

    cpr::Response forecast_response = cpr::Get(
        cpr::Url("https://api.open-meteo.com/v1/forecast"),
        cpr::Parameters{
            {"latitude", std::to_string(latitude)},
            {"longitude", std::to_string(longitude)},
            {"current",
             "temperature_2m,relative_humidity_2m,apparent_temperature,"
             "precipitation,wind_speed_10m,wind_direction_10m,weather_code"},
            {"hourly",
             "temperature_2m,relative_humidity_2m,apparent_temperature,"
             "precipitation,wind_speed_10m,wind_direction_10m,weather_code"},
            {"forecast_days", "16"},
            {"daily", "weather_code"}});
    std::string open_meteo_response = std::string(forecast_response.text);
    reader.parse(open_meteo_response, root);

    result[city]["current"]["temperature_2m"] =
        root["current"]["temperature_2m"];
    result[city]["current"]["relative_humidity_2m"] =
        root["current"]["relative_humidity_2m"];
    result[city]["current"]["apparent_temperature"] =
        root["current"]["apparent_temperature"];
    result[city]["current"]["precipitation"] = root["current"]["precipitation"];
    result[city]["current"]["wind_speed_10m"] =
        root["current"]["wind_speed_10m"];
    result[city]["current"]["wind_direction_10m"] =
        root["current"]["wind_direction_10m"];
    result[city]["current"]["weather_code"] = root["current"]["weather_code"];

    for (std::string name : root["hourly"].getMemberNames()) {
      for (int j = 0; j < 24 * 16; j += 24) {
        if (name != "time") {
          result[city]["daily"][root["hourly"]["time"][j].asString().substr(
              1, 9)]["night"][name] = root["hourly"][name][j];
          result[city]["daily"][root["hourly"]["time"][j + 6].asString().substr(
              1, 9)]["morning"][name] = root["hourly"][name][j + 6];
          result[city]["daily"]
                [root["hourly"]["time"][j + 12].asString().substr(1, 9)]["day"]
                [name] = root["hourly"][name][j + 12];
          result[city]["daily"]
                [root["hourly"]["time"][j + 18].asString().substr(1, 9)]
                ["evening"][name] = root["hourly"][name][j + 18];
        }
      }
    }
  }

  root_ = result;
  std::string json_string = result.toStyledString();
  output << json_string;
}
