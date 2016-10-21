/**
 * @file clickstream_hmm.cpp
 * Fits a hidden markov model with sequence observations to the extracted
 * sequences for students from a Coursera clickstream dump.
 */

#include <array>
#include <exception>

#include "json.hpp"

#include "meta/io/gzstream.h"
#include "meta/logging/logger.h"
#include "meta/sequence/hmm/hmm.h"
#include "meta/sequence/hmm/sequence_observations.h"
#include "meta/util/identifiers.h"

using namespace nlohmann;
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
    using action_sequence_type = std::vector<state_id>;
    using sequence_type = std::vector<action_sequence_type>;
    using training_data_type = std::vector<sequence_type>;

    std::vector<std::string> usernames;
    training_data_type train;
    uint64_t total_len = 0;
    std::string line;
    while (std::getline(std::cin, line))
    {
        auto obj = json::parse(line);
        usernames.push_back(obj["username"].get<std::string>());

        auto sequences = obj["sequences"];
        for (const auto& seq : sequences)
        {
            train.push_back(seq.get<sequence_type>());
            total_len += train.back().size();
        }
    }

    LOG(info) << "Training data consumed!" << ENDLG;
    LOG(info) << "Users: " << usernames.size() << ENDLG;
    LOG(info) << "Sequences: " << train.size() << ENDLG;
    LOG(info) << "Average length: "
              << static_cast<double>(total_len) / train.size() << ENDLG;

    std::mt19937 rng{47};

    using namespace hmm;

    const uint64_t num_states = 6;
    const uint64_t num_actions = 10;
    const double smoothing_constant = 1e-6;

    sequence_observations obs_dist{
        num_states, num_actions, rng,
        stats::dirichlet<state_id>{smoothing_constant, num_actions}};

    parallel::thread_pool pool;
    hidden_markov_model<sequence_observations> hmm{
        num_states, rng, std::move(obs_dist),
        stats::dirichlet<state_id>{smoothing_constant, num_states}};

    decltype(hmm)::training_options options;
    options.delta = 1e-4;
    options.max_iters = 50;

    LOG(info) << "Beginning training..." << ENDLG;
    hmm.fit(train, pool, options);

    LOG(info) << "Saving model..." << ENDLG;
    io::gzofstream output{"hmm-model.gz"};
    hmm.save(output);

    return 0;
}
