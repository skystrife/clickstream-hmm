cmake_minimum_required(VERSION 3.6.0)
project(clickstream-hmm)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(MeTA REQUIRED)

file(DOWNLOAD
        https://github.com/nlohmann/json/releases/download/v3.1.2/json.hpp
        ${PROJECT_SOURCE_DIR}/deps/json/json.hpp
     SHOW_PROGRESS
     EXPECTED_HASH
        SHA256=fbdfec4b4cf63b3b565d09f87e6c3c183bdd45c5be1864d3fcb338f6f02c1733
)
include_directories(include ${PROJECT_SOURCE_DIR}/deps/json)

add_executable(check-sorted src/check_sorted.cpp)
target_link_libraries(check-sorted meta-util)

add_executable(sort src/sort.cpp)
target_link_libraries(sort meta-util meta-io)

add_executable(extract-sequences src/extract_sequences.cpp)
target_link_libraries(extract-sequences meta-util meta-io)

add_executable(clickstream-hmm src/clickstream_hmm.cpp)
target_link_libraries(clickstream-hmm meta-sequence meta-hmm
    meta-stats)

add_executable(retrofit-hmm src/retrofit_hmm.cpp)
target_link_libraries(retrofit-hmm meta-sequence meta-hmm meta-stats
    meta-io)

add_executable(print-hmm src/print_hmm.cpp)
target_link_libraries(print-hmm meta-sequence meta-hmm)

add_executable(decode src/decode.cpp)
target_link_libraries(decode meta-sequence meta-hmm)

add_executable(classify-students src/classify_students.cpp)
target_link_libraries(classify-students meta-classify)

add_executable(rank-students src/rank_students.cpp)
target_link_libraries(rank-students meta-eval meta-stats)
