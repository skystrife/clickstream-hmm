/**
 * @file retrofit_hmm.cpp
 * Re-fits only the latent state transition probabilities to data, keeping
 * the latent state representation Markov models fixed.
 */

#include <array>
#include <exception>

#include "json.hpp"

#include "meta/io/gzstream.h"
#include "meta/logging/logger.h"
#include "meta/sequence/hmm/hmm.h"
#include "meta/sequence/hmm/sequence_observations.h"
#include "meta/stats/running_stats.h"
#include "meta/util/identifiers.h"

using namespace nlohmann;
using namespace meta;

#define META_DISABLE_HMM_OBSERVATION_UPDATE 1

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

    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " input output num_states" << std::endl;
        return 1;
    }

    uint64_t num_states = std::stoull(argv[1]);

    using namespace sequence;
    using action_sequence_type = std::vector<state_id>;
    using sequence_type = std::vector<action_sequence_type>;
    using training_data_type = std::vector<sequence_type>;

    std::vector<std::string> usernames;
    training_data_type train;

    stats::running_stats stats;
    std::string line;
    uint64_t total_sequences = 0;
    while (std::getline(std::cin, line))
    {
        auto obj = json::parse(line);
        usernames.push_back(obj["username"].get<std::string>());

        auto sequences = obj["sequences"].get<sequence_type>();
        train.push_back(sequences);

        for (const auto& seq : sequences)
            stats.add(seq.size());

        total_sequences += sequences.size();
    }

    LOG(info) << "Training data consumed!" << ENDLG;
    LOG(info) << "Users: " << usernames.size() << ENDLG;
    LOG(info) << "Sequences: " << total_sequences << ENDLG;
    LOG(info) << "Sequences per user: "
              << static_cast<double>(total_sequences) / usernames.size()
              << ENDLG;
    LOG(info) << "Average sequence length: " << stats.mean() << ENDLG;
    LOG(info) << "Variance of sequence length: " << stats.variance() << ENDLG;

    std::mt19937 rng{47};

    using namespace hmm;

    parallel::thread_pool pool;
    io::gzifstream input{argv[1]};
    hidden_markov_model<sequence_observations> hmm{input};

    decltype(hmm)::training_options options;
    options.delta = 1e-4;
    options.max_iters = 50;

    LOG(info) << "Beginning retrofitting..." << ENDLG;
    hmm.fit(train, pool, options);

    LOG(info) << "Saving modified model..." << ENDLG;
    io::gzofstream output{argv[2]};
    hmm.save(output);

    return 0;
}
