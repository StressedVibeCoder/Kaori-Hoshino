cmake_minimum_required(VERSION 3.20)
project(kaori VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(kaori
    src/main.cpp
    src/ollama_client.cpp
)

target_include_directories(kaori PRIVATE src)

target_link_libraries(kaori PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
)