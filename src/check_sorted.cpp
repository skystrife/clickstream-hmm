/**
 * @file check_sorted.cpp
 * Checks if a Coursera clickstream json file is sorted by timestamp
 */

#include <iostream>
#include <string>

#include "json.hpp"
#include "meta/util/progress.h"

using namespace nlohmann;

int main()
{
    uint64_t timestamp = 0;
    std::string line;
    for (uint64_t lineno = 0; std::getline(std::cin, line); ++lineno)
    {
        auto obj = json::parse(line);
        auto time = obj["timestamp"].get<uint64_t>();
        if (time < timestamp)
        {
            std::cout << "Unsorted at line " << lineno << std::endl;
            return 1;
        }
        timestamp = time;
    }
    std::cout << "All sorted!" << std::endl;
    return 0;
}
