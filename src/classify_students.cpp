/**
 * @file classify_students.cpp
 * Given two files produced by the decode application (one for "positive",
 * one for "negative"), run a simple classification experiment on them.
 */

#include "json.hpp"

#include "meta/classify/classifier/sgd.h"
#include "meta/logging/logger.h"

using namespace meta;
using namespace nlohmann;

int main(int argc, char** argv)
{
    logging::set_cerr_logging();

    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " positive.json negative.json"
                  << std::endl;
    }

    json pos_json;
    json neg_json;
    {
        std::ifstream pos_file{argv[1]};
        pos_file >> pos_json;

        std::ifstream neg_file{argv[2]};
        neg_file >> neg_json;
    }

    uint64_t total = pos_json.size() + neg_json.size();

    auto p_y1 = static_cast<double>(pos_json.size()) / total;
    auto p_y0 = static_cast<double>(neg_json.size()) / total;

    std::cout << "p(y = 1): " << p_y1 << "\n";
    std::cout << "p(y = 0): " << p_y0 << "\n";

    auto mi = 0.0;
    for (uint64_t i = 0; i < pos_json[0]["state_probs"].size(); ++i)
    {
        auto p_xi = 0.0;
        auto p_xi_y1 = 0.0;
        auto p_xi_y0 = 0.0;

        for (const auto& l : pos_json)
        {
            auto prob = l["state_probs"][i].get<double>();
            p_xi += prob;
            p_xi_y1 += prob;
        }

        for (const auto& l : neg_json)
        {
            auto prob = l["state_probs"][i].get<double>();
            p_xi += prob;
            p_xi_y0 += prob;
        }

        p_xi /= total;
        p_xi_y1 /= total;
        p_xi_y0 /= total;

        std::cout << "p(x = " << i << "): " << p_xi << "\n";
        std::cout << "p(x = " << i << ", y = 1): " << p_xi_y1 << "\n";
        std::cout << "p(x = " << i << ", y = 0): " << p_xi_y0 << "\n";


        mi += p_xi_y1 * std::log(p_xi_y1 / (p_xi * p_y1));
        mi += p_xi_y0 * std::log(p_xi_y0 / (p_xi * p_y0));
    }

    std::cout << "MI is " << mi << std::endl;

    return 0;
}
