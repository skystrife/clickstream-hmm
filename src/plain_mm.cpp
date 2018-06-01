/**
 * @file plain_mm.cpp
 * Fits a markov model to extracted sequences for students from a Coursera
 * clickstream dump.
 */

#include <array>
#include <exception>

#include "json.hpp"

#include "meta/io/gzstream.h"
#include "meta/logging/logger.h"
#include "meta/sequence/markov_model.h"
#include "meta/stats/running_stats.h"
#include "meta/util/identifiers.h"

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
} // namespace util
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

int main()
{
    logging::set_cerr_logging();

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

    const uint64_t num_actions = 10;
    const double smoothing_constant = 1e-6;

    markov_model::expected_counts_type counts{num_actions,
        stats::dirichlet<state_id>{smoothing_constant, num_actions}};

    for (const auto& user_seqs : train)
    {
        for (const auto& seq : user_seqs)
        {
            counts.increment(seq, 1.0);
        }
    }

    markov_model mm{std::move(counts)};
    auto arr = json::array();
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

    return 0;
}
