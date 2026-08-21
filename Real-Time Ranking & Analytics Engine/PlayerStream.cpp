#include "PlayerStream.hpp"

// Initializes the stream with the provided dataset and sets the read position to the beginning.
VectorPlayerStream::VectorPlayerStream(const std::vector<Player>& players)
    : players_(players)
    , current_index_(0)
{
}

// Returns the next player in the stream and advances the read position.
Player VectorPlayerStream::nextPlayer()
{
    // Prevent out-of-bounds memory access if the stream is exhausted.
    if (current_index_ >= players_.size()) {
        throw std::runtime_error("No more players remaining in the stream.");
    }
    return players_[current_index_++];
}

// Returns the number of unread players remaining in the stream.
size_t VectorPlayerStream::remaining() const
{
    return players_.size() - current_index_;
}
