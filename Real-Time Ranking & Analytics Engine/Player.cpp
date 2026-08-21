#include "Player.hpp"

// Initializes a new Player object.
Player::Player(const std::string& name, const size_t& level)
    : name_ { name }
    , level_ { level }
    , id_ { 0 }
{
}

// Compares players strictly by their game level.
bool Player::operator<(const Player& rhs) const
{
    return level_ < rhs.level_;
}

// Evaluates equality based solely on game level.
bool Player::operator==(const Player& rhs) const
{
    return level_ == rhs.level_;
}

// Evaluates if this player has a higher game level.
bool Player::operator>(const Player& rhs) const
{
    return level_ > rhs.level_;
}
