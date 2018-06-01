/**
 * @file print_plain_mm.cpp
 * Prints the distributions for a plain Markov model.
 */

#include <fstream>
#include <iostream>

#include "json.hpp"

#include "meta/io/gzstream.h"
#include "meta/logging/logger.h"
#include "meta/sequence/markov_model.h"

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

int main(int argc, char** argv)
{
    logging::set_cerr_logging();

    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " model file" << std::endl;
        return 1;
    }

    const char* filename = argv[1];

    using namespace sequence;

    std::ifstream input{filename, std::ios::binary};
    markov_model mm{input};

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
