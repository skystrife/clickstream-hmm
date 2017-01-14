/**
 * @file decode.cpp
 *
 * Given lists of sequences for individual students, computes the
 * (average) latent state probability and (average) latent state
 * transition probability for each student using the forward-backward
 * algorithm on a pre-trained HMM.
 */

#include "json.hpp"

#include "meta/io/gzstream.h"
#include "meta/logging/logger.h"
#include "meta/sequence/hmm/hmm.h"
#include "meta/sequence/hmm/sequence_observations.h"
#include "meta/util/identifiers.h"

using namespace nlohmann;
using namespace meta;

int main(int argc, char** argv)
{
    logging::set_cerr_logging();

    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " model.gz" << std::endl;
        return 1;
    }

    using namespace sequence;
    using namespace hmm;
    using action_sequence_type = std::vector<state_id>;
    using sequence_type = std::vector<action_sequence_type>;
    using hmm_type = hidden_markov_model<sequence_observations>;
    using fwdbwd = logarithm_forward_backward;

    io::gzifstream input{argv[1]};
    hmm_type hmm{input};

    auto arr = json::array();
    std::string line;
    while (std::getline(std::cin, line))
    {
        auto obj = json::parse(line);

        auto sequences = obj["sequences"].get<sequence_type>();

        // run forward-backward to get state and trans probabilities
        auto output_log_probs = fwdbwd::output_probabilities(hmm, sequences);

        auto fwd = fwdbwd::forward(hmm, sequences, output_log_probs);
        auto bwd = fwdbwd::backward(hmm, sequences, fwd, output_log_probs);

        auto log_gamma = fwdbwd::posterior_state_membership(hmm, fwd, bwd);

        std::vector<double> state_probs(hmm.num_states());
        for (state_id i{0}; i < hmm.num_states(); ++i)
        {
            state_probs[i] = 0.0;
            for (uint64_t t = 0; t < sequences.size(); ++t)
            {
                state_probs[i] += std::exp(log_gamma(t, i));
            }
        }

        auto denom
            = std::accumulate(state_probs.begin(), state_probs.end(), 0.0);
        std::transform(state_probs.begin(), state_probs.end(),
                       state_probs.begin(),
                       [=](double val) { return val / denom; });

        std::vector<std::vector<double>> transitions(hmm.num_states());
        for (label_id i{0}; i < hmm.num_states(); ++i)
        {
            state_id s_i{i};

            transitions[i].resize(hmm.num_states());

            for (label_id j{0}; j < hmm.num_states(); ++j)
            {
                state_id s_j{j};

                for (uint64_t t = 0; t < sequences.size() - 1; ++t)
                {
                    auto log_xi_tij
                        = log_gamma(t, s_i) + std::log(hmm.trans_prob(s_i, s_j))
                          + output_log_probs(t + 1, s_j)
                          + bwd.probability(t + 1, j) - bwd.probability(t, i);

                    transitions[i][j] += std::exp(log_xi_tij);
                }
            }

            auto denom = std::accumulate(transitions[i].begin(),
                                         transitions[i].end(), 0.0);
            std::transform(transitions[i].begin(), transitions[i].end(),
                           transitions[i].begin(),
                           [=](double val) { return val / denom; });
        }

        arr.push_back({{"username", obj["username"].get<std::string>()},
                       {"state_probs", state_probs},
                       {"transitions", transitions}});
    }

    std::cout << arr << "\n";

    return 0;
}
