/**
 * @file print_hmm.cpp
 * Prints the distributions for a HMM model file.
 */

#include "meta/io/gzstream.h"
#include "meta/sequence/hmm/hmm.h"
#include "meta/sequence/hmm/sequence_observations.h"

using namespace meta;

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

int main()
{
    logging::set_cerr_logging();

    using namespace sequence;
    using namespace hmm;

    io::gzifstream input{"hmm-model.gz"};
    hidden_markov_model<sequence_observations> hmm{input};

    const auto& obs_dist = hmm.observation_distribution();

    for (state_id sid{0}; sid < obs_dist.num_states(); ++sid)
    {
        std::cout << "HMM State " << sid << ":\n"
                  << "=========\n";
        const auto& mm = obs_dist.distribution(sid);

        std::cout << "Markov Model Initial probs:\n";
        for (state_id init{0}; init < mm.num_states(); ++init)
        {
            std::cout << "\"" << action_name(init) << "\":\t" << mm.initial_probability(init) << "\n";
        }
        std::cout << "\n";
        std::cout << "Markov Model Transition probs:\n";
        for (state_id i{0}; i < mm.num_states(); ++i)
        {
            std::cout << "\"" << action_name(i) << "\": [";
            for (state_id j{0}; j < mm.num_states(); ++j)
            {
                std::cout << mm.transition_probability(i, j);
                if (j + 1 < mm.num_states())
                    std::cout << ", ";
            }
            std::cout << "]\n";
        }
    }

    return 0;
}
