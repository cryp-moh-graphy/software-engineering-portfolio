#pragma once
#include "Player.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * @brief Interface for fetching Player objects sequentially.
 */
class PlayerStream {
public:
    virtual ~PlayerStream() = default;

    /**
     * @brief Retrieves the next Player in the stream.
     * @return The next Player object.
     * @post Advances the stream read position.
     * @throws std::runtime_error if the stream is exhausted.
     */
    virtual Player nextPlayer() = 0;

    /**
     * @brief Returns the number of unread players remaining in the stream.
     */
    virtual size_t remaining() const = 0;
};

/**
 * @brief A PlayerStream implementation backed by a std::vector.
 */
class VectorPlayerStream : public PlayerStream {
private:
    std::vector<Player> players_;
    size_t current_index_;

public:
    /**
     * @brief Initializes the stream with the provided dataset.
     * @param players The dataset to stream.
     */
    VectorPlayerStream(const std::vector<Player>& players);

    // Returns the next player in the stream and advances the read position.
    Player nextPlayer() override;

    // Returns the number of unread players remaining in the stream.
    size_t remaining() const override;
};
