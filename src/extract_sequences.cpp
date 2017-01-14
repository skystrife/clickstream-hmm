/**
 * @file extract_sequences.cpp
 * Extracts each browsing session for each user given a Coursera
 * clickstream dump.
 */

#include <iostream>
#include <regex>
#include <string>

#include "json.hpp"

#include "meta/hashing/probe_map.h"
#include "meta/io/filesystem.h"
#include "meta/io/packed.h"
#include "meta/logging/logger.h"
#include "meta/parallel/algorithm.h"
#include "meta/util/identifiers.h"
#include "meta/util/multiway_merge.h"
#include "meta/util/optional.h"
#include "meta/util/progress.h"

using namespace nlohmann;
using namespace meta;

MAKE_NUMERIC_IDENTIFIER_UDL(action_id, uint64_t, _aid)

using action_sequence = std::vector<action_id>;

struct student_record
{
    std::string username;
    std::vector<action_sequence> sequences;
};

struct memory_student_record
{
    uint64_t last_action_time = 0;
    std::vector<action_sequence> sequences;
};

util::optional<action_id> get_action(const std::string& str)
{
    static std::regex regexes[] = {
        std::regex{R"(\/forum\/list$)", std::regex_constants::ECMAScript},
        std::regex{R"(\/forum\/list\?forum_id=)",
                   std::regex_constants::ECMAScript},
        std::regex{R"(\/forum\/thread\?thread_id=)",
                   std::regex_constants::ECMAScript},
        std::regex{R"(\/forum\/search\?q=)", std::regex_constants::ECMAScript},
        std::regex{R"(\/forum\/posted_thread)",
                   std::regex_constants::ECMAScript},
        std::regex{R"(\/forum\/posted_reply)",
                   std::regex_constants::ECMAScript},
        // either: downloading the video or viewing it in the streaming player
        std::regex{R"(\/lecture(\/download|\?lecture_id=|\/view\?lecture_id=))",
                   std::regex_constants::ECMAScript},
        std::regex{R"(\/quiz\/start\?quiz_id=)",
                   std::regex_constants::ECMAScript},
        std::regex{R"(\/quiz\/submit)", std::regex_constants::ECMAScript},
        std::regex{R"(\/wiki)", std::regex_constants::ECMAScript}};

    action_id aid{0};
    for (const auto& regex : regexes)
    {
        if (std::regex_search(str, regex))
            return {aid};
        ++aid;
    }
    return util::nullopt;
}

using student_record_map
    = hashing::probe_map<std::string, memory_student_record>;

void insert_new_action(student_record_map& store, const std::string& username,
                       action_id aid, uint64_t timestamp)
{
    auto it = store.find(username);
    if (it == store.end())
        it = store.insert(username, memory_student_record{});

    // if the last action was more than 30 minutes ago, this is the start
    // of a new sequence
    if (timestamp > it->value().last_action_time + 30 * 60 * 1000)
    {
        it->value().sequences.emplace_back(1, aid);
    }
    // otherwise, this is just another action of the current sequence
    else
    {
        it->value().sequences.back().push_back(aid);
    }

    it->value().last_action_time = timestamp;
}

int main()
{
    logging::set_cerr_logging();

    student_record_map store;
    std::string line;
    while (std::getline(std::cin, line))
    {
        auto obj = json::parse(line);
        if (obj["key"].get<std::string>() != util::string_view{"pageview"})
            continue;
        auto username = obj["username"].get<std::string>();
        auto timestamp = obj["timestamp"].get<uint64_t>();
        // new dumps appear to set some cleaned url in "value"; use that if
        // we can
        if (auto action = get_action(obj["value"].get<std::string>()))
        {
            insert_new_action(store, username, *action, timestamp);
        }
        // otherwise use the value in page_url, which always exists
        else if (auto action = get_action(obj["page_url"].get<std::string>()))
        {
            insert_new_action(store, username, *action, timestamp);
        }
    }

    for (const auto& pr : store)
    {
        auto obj = json::object();
        obj["username"] = pr.key();
        obj["sequences"] = pr.value().sequences;

        std::cout << obj << '\n';
    }

    return 0;
}
