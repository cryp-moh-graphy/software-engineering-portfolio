#pragma once
#include <string>

struct Player {
    std::string name_;
    size_t level_;
    size_t id_;
    
    /**
     * @brief Initializes a new Player object.
     * @param name The name of the player.
     * @param level The current game level of the player.
     */
    Player(const std::string& name = "NONE", const size_t& level = 1);

    // Compares players strictly by their game level.
    bool operator<(const Player& rhs) const;
    
    // Evaluates equality based solely on game level.
    bool operator==(const Player& rhs) const;
    
    // Evaluates if this player has a higher game level.
    bool operator>(const Player& rhs) const;
};
