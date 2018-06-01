/**
 * @file print_hmm.cpp
 * Prints the distributions for a HMM model file.
 */

#include "json.hpp"

#include "meta/io/gzstream.h"
#include "meta/sequence/hmm/hmm.h"
#include "meta/sequence/hmm/sequence_observations.h"

using namespace nlohmann;
using namespace meta;

namespace meta
{
namespace util
{
template <class Tag, class T>
void to_json(json& j, const identifier<Tag, T>& i)
{
    j = static_cast<T>(i);
}

template <class Tag, class T>
void from_json(const json& j, identifier<Tag, T>& i)
{
    i = j.get<T>();
}
} // namespace sequence
} // namespace meta

util::string_view action_name(sequence::state_id aid)
{
    const static std::array<util::string_view, 10> actions
        = {{{"forum: list"},
            {"forum: thread list"},
            {"forum: thread view"},
            {"forum: search"},
            {"forum: post thread"},
            {"forum: post reply"},
            {"view lecture"},
            {"quiz: start"},
            {"quiz: submit"},
            {"wiki (course material)"}}};
    if (aid > actions.size())
        throw std::out_of_range{"invalid action id " + std::to_string(aid)};
    return actions[aid];
}

int main(int argc, char** argv)
{
    logging::set_cerr_logging();

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " human|json|json-trans"
                  << std::endl;
        return 1;
    }

    const char* filename = "hmm-model.gz";
    if (argc > 2)
        filename = argv[2];

    using namespace sequence;
    using namespace hmm;

    io::gzifstream input{filename};
    hidden_markov_model<sequence_observations> hmm{input};

    const auto& obs_dist = hmm.observation_distribution();

    if (argv[1] == util::string_view{"human"})
    {
        for (state_id sid{0}; sid < obs_dist.num_states(); ++sid)
        {
            std::cout << "HMM State " << sid << ":\n"
                      << "=========\n";
            const auto& mm = obs_dist.distribution(sid);

            std::cout << "Markov Model Initial probs:\n";
            for (state_id init{0}; init < mm.num_states(); ++init)
            {
                std::cout << "\"" << action_name(init) << "\":\t"
                          << mm.initial_probability(init) << "\n";
            }
            std::cout << "\n";
            std::cout << "Markov Model Transition probs:\n";
            for (state_id i{0}; i < mm.num_states(); ++i)
            {
                for (state_id j{0}; j < mm.num_states(); ++j)
                {
                    std::cout << action_name(i) << " -> " << action_name(j)
                              << ": " << mm.transition_probability(i, j)
                              << "\n";
                }
                std::cout << "\n";
            }
        }
    }
    else if (argv[1] == util::string_view{"json"})
    {
        for (state_id sid{0}; sid < obs_dist.num_states(); ++sid)
        {
            auto arr = json::array();

            const auto& mm = obs_dist.distribution(sid);
            for (state_id i{0}; i < mm.num_states(); ++i)
            {
                auto trans = json::array();
                for (state_id j{0}; j < mm.num_states(); ++j)
                {
                    trans.push_back(mm.transition_probability(i, j));
                }

                arr.push_back({{"name", action_name(i).to_string()},
                               {"init", mm.initial_probability(i)},
                               {"edges", trans}});
            }
            std::cout << arr << "\n";
        }
    }
    else if (argv[1] == util::string_view{"json-trans"})
    {
        auto arr = json::array();
        for (state_id i{0}; i < hmm.num_states(); ++i)
        {
            auto trans = json::array();
            for (state_id j{0}; j < hmm.num_states(); ++j)
            {
                trans.push_back(hmm.trans_prob(i, j));
            }

            arr.push_back(
                {{"name", i}, {"init", hmm.init_prob(i)}, {"edges", trans}});
        }
        std::cout << arr << "\n";
    }
    else
    {
        LOG(fatal) << "Unknown output format type" << ENDLG;
        return 1;
    }

    return 0;
}
