cmake_minimum_required(VERSION 3.12)

project(
    labwork7
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)


find_package(Curses REQUIRED)
include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 3b15fa82ea74739b574d705fea44959b58142eb8) 
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)


FetchContent_Declare(jsoncpp URL https://github.com/open-source-parsers/jsoncpp/archive/refs/tags/1.9.5.tar.gz)
FetchContent_MakeAvailable(jsoncpp)

FetchContent_Declare(ftxui GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
                           GIT_TAG v5.0.0
)

FetchContent_MakeAvailable(ftxui)

add_executable(labwork7 lib/App.cpp lib/ConfigParser.cpp lib/ForecastManager.cpp)

target_link_libraries(labwork7 PRIVATE cpr::cpr)
target_link_libraries(labwork7 PRIVATE jsoncpp_lib)
target_link_libraries(labwork7 PRIVATE nlohmann_json::nlohmann_json)

target_link_libraries(labwork7
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)
target_link_libraries(labwork7 PRIVATE ${CURSES_LIBRARIES})
