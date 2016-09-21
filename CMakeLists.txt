cmake_minimum_required(VERSION 3.2.0)
project(clickstream-hmm)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(MeTA REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(check-sorted src/check_sorted.cpp)
target_link_libraries(check-sorted nlohmann_json meta-util)
