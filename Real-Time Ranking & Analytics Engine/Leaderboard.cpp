#include "Leaderboard.hpp"
#include <algorithm>
#include <chrono>
#include <functional>

RankingResult::RankingResult(const std::vector<Player>& top, const std::unordered_map<size_t, size_t>& cutoffs, double elapsed)
    : top_ { top }
    , cutoffs_ { cutoffs }
    , elapsed_ { elapsed }
{
}

namespace Offline {

RankingResult quickSelectRank(std::vector<Player>& players)
{
    // Start the clock at the entry point to capture total algorithmic runtime.
    auto start = std::chrono::high_resolution_clock::now();

    // Calculate the top 10% cutoff. Integer division naturally handles truncation.
    size_t top_count = players.size() / 10;

    // Handle edge case: dataset is too small to yield a top 10%.
    if (top_count == 0) {
        auto end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(end - start).count();
        return RankingResult({}, {}, elapsed);
    }

    // Determine the pivot index for the partition boundary.
    size_t target_index = players.size() - top_count;

    // Use QuickSelect (O(N) average time) to partition the vector. 
    // Elements at and beyond target_index will be the top 10%.
    std::nth_element(players.begin(), players.begin() + target_index, players.end());

    // Sort only the isolated top 10% slice to meet the ascending order requirement.
    std::sort(players.begin() + target_index, players.end());

    // Extract the validated leaderboard tier.
    std::vector<Player> top_players(players.begin() + target_index, players.end());

    // Stop timer and calculate total duration in milliseconds.
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();

    // Return the results. Offline algorithms do not use the cutoffs map.
    return RankingResult(top_players, {}, elapsed);
}

RankingResult heapRank(std::vector<Player>& players)
{
    // Start performance tracking timer.
    auto start = std::chrono::high_resolution_clock::now();

    // Calculate the top 10% target size.
    size_t top_count = players.size() / 10;

    // Handle edge case: dataset is too small to yield a top 10%.
    if (top_count == 0) {
        auto end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(end - start).count();
        return RankingResult({}, {}, elapsed);
    }

    // Convert the vector into a max-heap in O(N) time.
    std::make_heap(players.begin(), players.end());

    // Extract the top_count largest elements.
    for (size_t i = 0; i < top_count; ++i) {
        std::pop_heap(players.begin(), players.end() - i);
    }

    // Extract the largest elements, which pop_heap placed at the back of the vector.
    std::vector<Player> top_players(players.end() - top_count, players.end());

    // Stop execution clock before return overhead.
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();

    return RankingResult(top_players, {}, elapsed);
}

} // namespace Offline

namespace Online {

void replaceMin(PlayerIt first, PlayerIt last, Player& target)
{
    // Return immediately if the range is empty.
    if (first == last) return;

    // Overwrite the root element directly to avoid deletion/reinsertion overhead.
    *first = std::move(target);

    // Perform heapify-down to restore the min-heap property.
    size_t index = 0;
    size_t count = std::distance(first, last);

    while (true) {
        size_t left_child = 2 * index + 1;
        size_t right_child = 2 * index + 2;
        size_t smallest = index;

        // Check if the left child is smaller than the current node.
        if (left_child < count && *(first + left_child) < *(first + smallest)) {
            smallest = left_child;
        }
        // Check if the right child is the smallest.
        if (right_child < count && *(first + right_child) < *(first + smallest)) {
            smallest = right_child;
        }

        // If a child is smaller, swap and continue sinking the node.
        // Otherwise, the min-heap property is restored.
        if (smallest != index) {
            std::swap(*(first + index), *(first + smallest));
            index = smallest;
        } else {
            break;
        }
    }
}

RankingResult rankIncoming(PlayerStream& stream, const size_t& reporting_interval)
{
    std::vector<Player> top_heap;
    std::unordered_map<size_t, size_t> cutoffs;
    double total_elapsed_ms = 0.0;
    size_t processed_count = 0;

    // Process all players provided by the stream.
    while (stream.remaining() > 0) {
        // Fetch the next player *before* starting the timer to exclude I/O latency.
        Player p = stream.nextPlayer();

        auto start = std::chrono::high_resolution_clock::now();
        processed_count++;

        // Phase 1: Fill the leaderboard buffer until it reaches the reporting interval.
        if (top_heap.size() < reporting_interval) {
            top_heap.push_back(p);
            
            // Convert the buffer into a min-heap once it reaches capacity.
            if (top_heap.size() == reporting_interval) {
                std::make_heap(top_heap.begin(), top_heap.end(), std::greater<Player>());
            }
        } else {
            // Phase 2: Buffer is full. Filter incoming entries against the active minimum (root).
            if (p.level_ > top_heap.front().level_) {
                replaceMin(top_heap.begin(), top_heap.end(), p);
            }
        }

        // Milestone Tracking: Record the current cutoff requirement at each interval.
        if (processed_count % reporting_interval == 0) {
            if (!top_heap.empty()) {
                cutoffs[processed_count] = top_heap.front().level_;
            }
        }

        // Add the current step's duration to the cumulative runtime tally.
        auto end = std::chrono::high_resolution_clock::now();
        total_elapsed_ms += std::chrono::duration<double, std::milli>(end - start).count();
    }

    // Terminal Logging: Log a final cutoff entry after processing all players.
    auto final_start = std::chrono::high_resolution_clock::now();
    if (processed_count > 0 && !top_heap.empty()) {
        if (top_heap.size() < reporting_interval) {
            cutoffs[processed_count] = std::min_element(top_heap.begin(), top_heap.end())->level_;
        } else {
            cutoffs[processed_count] = top_heap.front().level_;
        }
    }

    // Ensure the final leaderboard is in strict ascending order.
    std::sort(top_heap.begin(), top_heap.end());

    // Measure final terminal operations and append to total time.
    auto final_end = std::chrono::high_resolution_clock::now();
    total_elapsed_ms += std::chrono::duration<double, std::milli>(final_end - final_start).count();

    return RankingResult(top_heap, cutoffs, total_elapsed_ms);
}

} // namespace Online
