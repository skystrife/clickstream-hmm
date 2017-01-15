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

add_executable(sort src/sort.cpp)
target_link_libraries(sort nlohmann_json meta-util meta-io)

add_executable(extract-sequences src/extract_sequences.cpp)
target_link_libraries(extract-sequences nlohmann_json meta-util meta-io)

add_executable(clickstream-hmm src/clickstream_hmm.cpp)
target_link_libraries(clickstream-hmm nlohmann_json meta-sequence meta-hmm
    meta-stats)

add_executable(retrofit-hmm src/retrofit_hmm.cpp)
target_link_libraries(retrofit-hmm nlohmann_json meta-sequence meta-hmm
    meta-stats meta-io)

add_executable(print-hmm src/print_hmm.cpp)
target_link_libraries(print-hmm nlohmann_json meta-sequence meta-hmm)

add_executable(decode src/decode.cpp)
target_link_libraries(decode nlohmann_json meta-sequence meta-hmm)

add_executable(classify-students src/classify_students.cpp)
target_link_libraries(classify-students nlohmann_json meta-classify)

add_executable(rank-students src/rank_students.cpp)
target_link_libraries(rank-students nlohmann_json meta-eval meta-stats)
