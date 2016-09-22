/**
 * @file sort.cpp
 * Sorts a Coursera clickstream json file by the timestamp key.
 */

#include <iostream>
#include <string>

#include "json.hpp"

#include "meta/hashing/probe_map.h"
#include "meta/io/filesystem.h"
#include "meta/io/packed.h"
#include "meta/logging/logger.h"
#include "meta/parallel/algorithm.h"
#include "meta/util/multiway_merge.h"

using namespace nlohmann;
using namespace meta;

struct line_record
{
    line_record(uint64_t timestamp, uint64_t byte_pos)
        : timestamp_{timestamp}, byte_pos_{byte_pos}
    {
        // nothing
    }

    uint64_t timestamp_;
    uint64_t byte_pos_;
};

bool operator<(const line_record& a, const line_record& b)
{
    return a.timestamp_ < b.timestamp_;
}

struct full_line_record
{
    uint64_t timestamp;
    std::string line;

    void merge_with(full_line_record&&)
    {
    }
};

bool operator<(const full_line_record& a, const full_line_record& b)
{
    return a.timestamp < b.timestamp;
}

bool operator==(const full_line_record&, const full_line_record&)
{
    return false;
}

template <class InputStream>
uint64_t packed_read(InputStream& in, full_line_record& flr)
{
    using io::packed::read;
    return read(in, flr.timestamp) + read(in, flr.line);
}

void flush_chunk(uint64_t chunk_num, std::vector<line_record>& lines,
                 const std::vector<char>& buffer)
{
    LOG(info) << "Sorting chunk " << chunk_num + 1 << " of size "
              << lines.size() << "..." << ENDLG;
    filesystem::make_directory("tmp");
    std::ofstream chunk{"tmp/chunk-" + std::to_string(chunk_num),
                        std::ios::binary};

    parallel::thread_pool pool;
    parallel::sort(lines.begin(), lines.end(), pool);

    LOG(info) << "Flushing chunk " << chunk_num + 1 << "..." << ENDLG;
    for (const auto& rec : lines)
    {
        util::string_view sv{buffer.data() + rec.byte_pos_};
        io::packed::write(chunk, rec.timestamp_);
        io::packed::write(chunk, sv);
    }
    LOG(info) << "Flushed chunk " << chunk_num + 1 << ENDLG;
}

int main(int argc, char** argv)
{
    uint64_t max_ram = 1024u * 1024 * 1024 * 8; // 8 GB
    if (argc >= 2)
        max_ram = 1024u * 1024 * 1024 * std::stoul(argv[1]);

    logging::set_cerr_logging();

    LOG(info) << "Attempting to use no more than about "
              << max_ram / (1024 * 1024 * 1024.0) << " GB of RAM" << ENDLG;

    std::vector<char> buffer(max_ram);
    std::vector<line_record> lines;

    std::string line;
    uint64_t num_chunks = 0;
    char* pos = buffer.data();
    while (std::getline(std::cin, line))
    {
        // out of room, so flush a chunk to disk
        if (pos + line.size() + 1 >= buffer.data() + buffer.size())
        {
            flush_chunk(num_chunks++, lines, buffer);

            lines.clear();
            std::fill(buffer.begin(), buffer.end(), 0);
            pos = buffer.data();
        }

        auto obj = json::parse(line);
        lines.emplace_back(obj["timestamp"].get<uint64_t>(),
                           static_cast<uint64_t>(pos - buffer.data()));

        // add line to in-memory buffer
        std::copy(line.begin(), line.end() + 1, pos);
        pos += line.size() + 1;
    }

    if (num_chunks > 0)
    {
        if (pos != buffer.data())
            flush_chunk(num_chunks++, lines, buffer);

        std::vector<util::chunk_iterator<full_line_record>> chunks;
        chunks.reserve(num_chunks);
        for (uint64_t i = 0; i < num_chunks; ++i)
            chunks.emplace_back("tmp/chunk-" + std::to_string(i));

        util::multiway_merge(
            chunks.begin(), chunks.end(),
            [&](full_line_record&& flr) { std::cout << flr.line << "\n"; });
    }
    else
    {
        LOG(info) << "Sorting " << lines.size() << " records in memory..."
                  << ENDLG;
        parallel::thread_pool pool;
        parallel::sort(lines.begin(), lines.end(), pool);

        LOG(info) << "Writing..." << ENDLG;
        for (const auto& line : lines)
            std::cout << util::string_view{buffer.data() + line.byte_pos_}
                      << "\n";
    }

    return 0;
}
