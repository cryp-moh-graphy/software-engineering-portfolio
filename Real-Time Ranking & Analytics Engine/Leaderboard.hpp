#pragma once

#include "Player.hpp"
#include "PlayerStream.hpp"

#include <iterator>
#include <unordered_map>
#include <vector>

// Bundles grading metrics and sorting outputs.
struct RankingResult {
    // Top-performing players sorted in ascending order.
    std::vector<Player> top_;

    // Maps processing milestones to required cutoff levels.
    std::unordered_map<size_t, size_t> cutoffs_;

    // Computational execution time in milliseconds.
    double elapsed_;

    // Constructs a RankingResult with optional default values for early edge-case exits.
    RankingResult(const std::vector<Player>& top = {}, const std::unordered_map<size_t, size_t>& cutoffs = {}, double elapsed = 0);
};

// Batch-processing algorithms requiring the full dataset upfront.
namespace Offline {
    // Isolates the top 10% of players using an average O(N) QuickSelect routine.
    RankingResult quickSelectRank(std::vector<Player>& players);

    // Extracts the top 10% of players sequentially using a max-heap structure.
    RankingResult heapRank(std::vector<Player>& players);
};

// Streaming operations for sequential data processing.
namespace Online {
    using PlayerIt = std::vector<Player>::iterator;

    // Overwrites the min-heap root and performs heapify-down to maintain heap properties.
    void replaceMin(PlayerIt first, PlayerIt last, Player& target);

    // Processes a live stream to dynamically track top players and milestone cutoffs.
    RankingResult rankIncoming(PlayerStream& stream, const size_t& reporting_interval);
};
