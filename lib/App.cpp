#include "App.h"

#include <sys/termios.h>
#include <unistd.h>

#include <cstdlib>

#include "ConfigParser.h"
using namespace ftxui;

void App::Run(const std::string& config_path) {
  system("clear");

  bool is_running = true;
  Config config;
  ForecastManager writer;
  int y_wsize = 32;
  int current_city = 0;
  char c;
  config.Parse(config_path);

  writer.WriteForecast(config);

  day_count = config.GetForecastDayCount();
  std::vector<std::string> cities = writer.GetCities();

  struct termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  Element doc = BuildUI(config, cities[current_city]);
  auto screen = Screen::Create(Dimension::Full(), Dimension::Fixed(y_wsize));
  Render(screen, doc);
  screen.Print();

  while (is_running) {
    std::cin.get(c);
    if (c == char(27)) {
      is_running = false;
    } else if (c == '=') {
      if (day_count < 16) {
        system("clear");
        screen.Clear();

        ++day_count;
        doc = BuildUI(config, cities[current_city]);
        y_wsize += 10;
        screen = Screen::Create(Dimension::Full(), Dimension::Fixed(y_wsize));

        Render(screen, doc);
        screen.Print();
      }

    } else if (c == '-') {
      if (day_count > 1) {
        system("clear");

        screen.Clear();
        --day_count;
        doc = BuildUI(config, cities[current_city]);
        y_wsize -= 10;
        screen = Screen::Create(Dimension::Full(), Dimension::Fixed(y_wsize));

        Render(screen, doc);
        screen.Print();
      }
    } else if (c == 'n') {
      if (current_city < cities.size() - 1) {
        ++current_city;
        system("clear");

        screen.Clear();
        doc = BuildUI(config, cities[current_city]);
        screen = Screen::Create(Dimension::Full(), Dimension::Fixed(y_wsize));

        Render(screen, doc);
        screen.Print();
      }
    } else if (c == 'p') {
      if (current_city > 0) {
        --current_city;

        system("clear");

        screen.Clear();
        doc = BuildUI(config, cities[current_city]);
        screen = Screen::Create(Dimension::Full(), Dimension::Fixed(y_wsize));

        Render(screen, doc);
        screen.Print();
      }
    }
  }
}

Element App::BuildUI(Config& config, std::string& city_) {
  Json::Value root;
  Json::CharReaderBuilder builder;
  JSONCPP_STRING errors;
  std::ifstream data("forecast.json");

  if (!Json::parseFromStream(builder, data, &root, &errors)) {
    std::cerr << errors << '\n';
    exit(EXIT_FAILURE);
  }

  std::vector<Element> result;

  Element current_cloud;
  Element current_summary;

  for (std::string city : root.getMemberNames()) {
    if (city == city_) {
      current_cloud = GetCloud(root[city]["current"]["weather_code"].asInt());
      current_summary = FormSummary(root[city]["current"]);
    }
  }

  result.push_back(
      vbox({text(city_), hbox({current_cloud, text("  "), current_summary})}));

  Element doc;
  int in = 0;
  for (std::string city : root.getMemberNames()) {
    if (city == city_) {
      for (std::string type : root[city].getMemberNames()) {
        if (type == "daily") {
          for (std::string date : root[city][type].getMemberNames()) {
            doc = vbox(
                {text(date),
                 vbox(
                     {hbox({filler(),
                            text("Night____Morning____Afternoon____Evening"),
                            filler()}),
                      separator(),
                      hbox({
                          GetCloud(
                              root[city][type][date]["night"]["weather_code"]
                                  .asInt()),
                          text("  "),
                          FormSummary(root[city][type][date]["night"]),
                          separatorLight(),
                          GetCloud(
                              root[city][type][date]["morning"]["weather_code"]
                                  .asInt()),
                          text("  "),
                          FormSummary(root[city][type][date]["morning"]),
                          separatorLight(),
                          GetCloud(root[city][type][date]["day"]["weather_code"]
                                       .asInt()),
                          text("  "),
                          FormSummary(root[city][type][date]["day"]),
                          separatorLight(),
                          GetCloud(
                              root[city][type][date]["evening"]["weather_code"]
                                  .asInt()),
                          text("  "),
                          FormSummary(root[city][type][date]["evening"]),
                      })}) |
                     borderRounded});

            result.push_back(doc);
            ++in;
            if (in == day_count) {
              in = 0;
              break;
            }
          }
        }
      }
    }
  }

  Element res = vbox(result);
  return res;
}

Element App::FormSummary(Json::Value value) {
  Element result;

  result = vbox(
      {text(GetDescOfWWOCodes(value["weather_code"].asInt())),
       text(RoundFloat(value["temperature_2m"].asString()) + " (" +
            RoundFloat(value["apparent_temperature"].asString()) + ")"),
       text(GetWindDirection(value["wind_direction_10m"].asString()) + ' ' +
            RoundFloat(value["wind_speed_10m"].asString() + " km/h")),
       text(RoundFloat(value["precipitation"].asString()) + " mm")});

  return result;
}

Element App::GetCloud(const uint8_t weather_code) {
  Element cloud;
  if (weather_code == 0 || weather_code == 1) {
    cloud = vbox({text("   \\  |  /"), text(" \'-.; ; ;.-\'"),
                  text("-==; ; ; ;==-"), text(" .-' ; ; ;'-."),
                  text("   /  |  \\")});
  } else if (weather_code == 2) {
    cloud = vbox({text("  \\    /"), text("_ / \"\".-."), text("  \\ _(   )."),
                  text("  /(__ _(__)")});
  } else if (weather_code == 3) {
    cloud = vbox({
        text("    .---."),
        text(" .-(     )."),
        text("(___.__)_ _)"),
    });
  } else if (weather_code == 45 || weather_code == 48) {
    cloud = vbox({
        text("_ - _ - _ -"),
        text(" _ - _ - _"),
        text("_ - _ - _ -"),
    });
  } else if (weather_code == 56 || weather_code == 57) {
    cloud = vbox({text("  .-. "), text(" (   )."), text("(___(__)"),
                  text(" ‘ ‘ ‘ ‘"), text("‘ ‘ ‘ ‘")});
  } else if (weather_code == 61 || weather_code == 63 || weather_code == 65) {
    cloud = vbox({text("  .-. "), text(" (   )."), text("(___(__)"),
                  text(" ‘ ‘ ‘ ‘"), text("‘ ‘ ‘ ‘")});
  } else if (weather_code == 66 || weather_code == 67) {
    cloud = vbox({text("  .-. "), text(" (   )."), text("(___(__)"),
                  text(" \' * \' *"), text("* \' * \'")});
  } else if (weather_code == 71 || weather_code == 73 || weather_code == 75 ||
             weather_code == 77) {
    cloud = vbox({text("  .-. "), text(" (   )."), text("(___(__)"),
                  text(" * * * *"), text("* * * *")});
  } else if (weather_code == 80 || weather_code == 81 || weather_code == 82) {
    cloud = vbox({text("  .-. "), text(" (   )."), text("(___(__)"),
                  text(" / / / /"), text("/ / / /")});
  } else if (weather_code == 85 || weather_code == 86) {
    cloud = vbox({text("  .-. "), text(" (   )."), text("(___(__)"),
                  text(" / * / *"), text("* / * /")});
  } else if (weather_code == 95 || weather_code == 96 || weather_code == 99) {
    cloud = vbox({text("  .-. "), text(" (   )."), text("(___(__)"),
                  text("/_ /_ /_"), text(" /  /  /")});
  } else {
    cloud = vbox({text("  .-. "), text(" (   )."), text("(___(__)"),
                  text(" ‘ ‘ ‘ ‘"), text("‘ ‘ ‘ ‘")});
  }

  return cloud;
}

std::string App::RoundFloat(const std::string& temperature) {
  std::string result = temperature;
  size_t decimal_pos = result.find('.');
  if (decimal_pos != std::string::npos) {
    result = result.substr(0, decimal_pos + 2);
  }

  return result;
}

std::string App::GetDescOfWWOCodes(const uint8_t& weather_code) {
  if (weather_code == 0) {
    return "Clear sky";
  } else if (weather_code == 1) {
    return "Mainly clear";
  } else if (weather_code == 2) {
    return "Partly cloudy";
  } else if (weather_code == 3) {
    return "Overcast";
  } else if (weather_code == 45) {
    return "Fog";
  } else if (weather_code == 48) {
    return "Depositing rime fog";
  } else if (weather_code == 51) {
    return "Drizzle light";
  } else if (weather_code == 53) {
    return "Drizzle moderate";
  } else if (weather_code == 55) {
    return "Drizzle dense";
  } else if (weather_code == 56) {
    return "Freezing drizzle light";
  } else if (weather_code == 57) {
    return "Freezing drizzle dense";
  } else if (weather_code == 61) {
    return "Rain slight";
  } else if (weather_code == 63) {
    return "Rain moderate";
  } else if (weather_code == 65) {
    return "Rain heavy";
  } else if (weather_code == 66) {
    return "Freezing rain light";
  } else if (weather_code == 67) {
    return "Freezing rain heavy";
  } else if (weather_code == 71) {
    return "Snow fall slight";
  } else if (weather_code == 73) {
    return "Snow fall moderate";
  } else if (weather_code == 75) {
    return "Snow fall heavy";
  } else if (weather_code == 77) {
    return "Snow grains";
  } else if (weather_code == 80) {
    return "Rain showers slight";
  } else if (weather_code == 81) {
    return "Rain showers moderate";
  } else if (weather_code == 82) {
    return "Rain showers heavy";
  } else if (weather_code == 85) {
    return "Snow showers slight";
  } else if (weather_code == 86) {
    return "Snow showers heavy";
  } else if (weather_code == 95) {
    return "Thunderstorm";
  } else if (weather_code == 96) {
    return "Thunderstorm with slight hail";
  } else if (weather_code == 99) {
    return "Thunderstorm with heavy hail";
  }
}

std::string App::GetWindDirection(const std::string& degrees) {
  uint16_t degrees_int = std::stoi(degrees);
  if (degrees_int >= 337.5 || degrees_int < 22.5) {
    return "↑";
  } else if (degrees_int >= 22.5 && degrees_int < 67.5) {
    return "↗";
  } else if (degrees_int >= 67.5 && degrees_int < 112.5) {
    return "→";
  } else if (degrees_int >= 112.5 && degrees_int < 157.5) {
    return "↘";
  } else if (degrees_int >= 157.5 && degrees_int < 202.5) {
    return "↓";
  } else if (degrees_int >= 202.5 && degrees_int < 247.5) {
    return "↙";
  } else if (degrees_int >= 247.5 && degrees_int < 292.5) {
    return "←";
  } else if (degrees_int >= 292.5 && degrees_int < 337.5) {
    return "↖";
  }
}

int main() {
  std::string path = "/Users/deniv/labwork7-Mixturka/lib/config.json";
  App app;
  app.Run(path);
  return EXIT_SUCCESS;
}
