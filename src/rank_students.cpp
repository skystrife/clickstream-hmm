/**
 * @file rank_students.cpp
 * Given a file of decoded sequences and an "ideal" ranking, computes the
 * rank correlation between the ranked list of students by preference for
 * a certain state and the ideal ranking.
 */

#include "json.hpp"

#include "meta/classify/classifier/sgd.h"
#include "meta/hashing/probe_map.h"
#include "meta/index/eval/rank_correlation.h"
#include "meta/logging/logger.h"
#include "meta/stats/running_stats.h"

using namespace meta;
using namespace nlohmann;

int main(int argc, char** argv)
{
    logging::set_cerr_logging();

    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " features.json ideal_rank.tsv"
                  << std::endl;
    }

    json feats;
    {
        std::ifstream feats_file{argv[1]};
        feats_file >> feats;
    }

    hashing::probe_map<std::string, double> grades;
    {
        std::ifstream grades_file{argv[2]};
        std::string username;
        double grade;
        while (grades_file >> username >> grade)
        {
            grades[username] = grade;
        }
    }

    std::vector<double> ideal(feats.size());
    std::vector<double> candidate(feats.size());

    for (uint64_t s = 0; s < feats[0]["state_probs"].size(); ++s)
    {
        uint64_t i = 0;

        hashing::probe_map<std::string, double> prob;

        std::vector<std::string> all_students;
        all_students.reserve(feats.size());
        for (const auto& j : feats)
        {
            auto username = j["username"].get<std::string>();
            ideal[i] = grades[username];
            candidate[i] = j["state_probs"][s];

            prob[username] = candidate[i];

            all_students.push_back(username);

            ++i;
        }

        std::sort(all_students.begin(), all_students.end(),
                  [&](const std::string& u1, const std::string& u2) {
                      return prob[u2] < prob[u1];
                  });

        stats::running_stats perfect;
        stats::running_stats low;
        i = 0;
        for (const auto& user : all_students)
        {
            if (grades[user] >= 10)
            {
                perfect.add(i);
            }
            else if (grades[user] <= 7)
            {
                low.add(i);
            }

            i++;
        }

        index::rank_correlation rnk{candidate, ideal};
        std::cout << "tau_b(" << s << "): " << rnk.tau_b() << "\n";
        std::cout << "NDPM(" << s << "): " << rnk.ndpm() << "\n";
        std::cout << "avg rank(" << s << ", perfect): " << perfect.mean()
                  << ", SD: " << perfect.stddev() << ", N: " << perfect.size()
                  << "\n";
        std::cout << "avg rank(" << s << ", low): " << low.mean()
                  << ", SD: " << low.stddev() << ", N: " << low.size()
                  << "\n\n";
    }

    uint64_t i = 0;
    hashing::probe_map<std::string, double> prob;
    std::vector<std::string> all_students;
    all_students.reserve(feats.size());
    for (const auto& j : feats)
    {
        auto username = j["username"].get<std::string>();
        ideal[i] = grades[username];

        if (j["transitions"][2][2].is_null())
        {
            candidate[i] = 0;
        }
        else
        {
            candidate[i] = j["transitions"][2][2];
        }

        prob[username] = candidate[i];

        all_students.push_back(username);

        ++i;
    }

    std::sort(all_students.begin(), all_students.end(),
              [&](const std::string& u1, const std::string& u2) {
                  return prob[u2] < prob[u1];
              });

    stats::running_stats perfect;
    stats::running_stats low;
    i = 0;
    for (const auto& user : all_students)
    {
        if (grades[user] >= 10)
        {
            perfect.add(i);
        }
        else if (grades[user] <= 7)
        {
            low.add(i);
        }

        ++i;
    }

    index::rank_correlation rnk{candidate, ideal};
    std::cout << "tau_b("
              << "2, 2"
              << "): " << rnk.tau_b() << "\n";
    std::cout << "NDPM("
              << "2, 2"
              << "): " << rnk.ndpm() << "\n";
    std::cout << "avg rank("
              << "2, 2"
              << ", perfect): " << perfect.mean()
              << ", SD: " << perfect.stddev() << ", N: " << perfect.size()
              << "\n";
    std::cout << "avg rank("
              << "2, 2"
              << ", low): " << low.mean() << ", SD: " << low.stddev()
              << ", N: " << low.size() << "\n\n";

    return 0;
}
