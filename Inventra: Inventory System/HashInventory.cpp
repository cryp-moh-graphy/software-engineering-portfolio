#include "HashInventory.hpp"

/**
 * Default constructor initializes:
 * - empty set
 * - no equipped item
 * - weight = 0
 */
template <class Comparator>
Inventory<Comparator, std::unordered_set<Item>>::Inventory()
    : items_ {}, equipped_ { nullptr }, weight_ { 0.0 } {}

/**
 * Returns pointer to equipped item
 */
template <class Comparator>
Item* Inventory<Comparator, std::unordered_set<Item>>::getEquipped() const {
    return equipped_;
}

/**
 * Equip item (just assign pointer)
 */
template <class Comparator>
void Inventory<Comparator, std::unordered_set<Item>>::equip(Item* itemToEquip) {
    equipped_ = itemToEquip;
}

/**
 * Delete equipped item safely
 */
template <class Comparator>
void Inventory<Comparator, std::unordered_set<Item>>::discardEquipped() {
    if (!equipped_) return;
    delete equipped_;
    equipped_ = nullptr;
}

/**
 * Return total weight
 */
template <class Comparator>
float Inventory<Comparator, std::unordered_set<Item>>::getWeight() const {
    return weight_;
}

/**
 * Return number of items
 */
template <class Comparator>
size_t Inventory<Comparator, std::unordered_set<Item>>::size() const {
    return items_.size();
}

/**
 * Return copy of items
 */
template <class Comparator>
std::unordered_set<Item>
Inventory<Comparator, std::unordered_set<Item>>::getItems() const {
    return items_;
}

/**
 * Insert if not duplicate
 */
template <class Comparator>
bool Inventory<Comparator, std::unordered_set<Item>>::pickup(const Item& target) {
    if (items_.count(target)) return false;
    items_.insert(target);
    weight_ += target.weight_;
    return true;
}

/**
 * @brief Attempts to remove an item from the inventory by name.
 *
 * @param name Name of the item to be removed
 * @return true if the item was successfully removed, false if the
 *         item was not found in the inventory
 * @post Updates the weight_ member to reflect removing the Item
 */
template <class Comparator>
bool Inventory<Comparator, std::unordered_set<Item>>::discard(const std::string& name) {
    for (auto it = items_.begin(); it != items_.end(); ++it) {
        if (it->name_ == name) {
            weight_ -= it->weight_;
            items_.erase(it);
            return true;
        }
    }
    return false;
}

/**
 * @brief Checks if an item with the given name exists in the inventory.
 *
 * @param name Name of the item to search for
 * @return true if the item exists in the inventory, false otherwise
 */

  // template <class Comparator>
  // bool Inventory<Comparator, std::unordered_set<Item>>::contains(const std::string& name) const {
  //     for (const auto& i : items_) {
  //         if (i.name_ == name) return true;
  //     }
  //     return false;
  // }
template <class Comparator>
bool Inventory<Comparator, std::unordered_set<Item>>::contains(const std::string& itemName) const {
    // WRONG: for (auto& i : items_) { ... } -> This is O(N)
    // RIGHT: jump straight to the bucket using the hash
    return items_.find(Item(itemName)) != items_.end();
}

/**
 * @brief Queries the inventory for items within a specified range.
 *
 * Returns a set of items that fall between the start and end items
 * according to the specified Comparator (inclusive on both ends)
 *
 * @param start An Item whose compared property is the lower bound of the query range
 * @param end An Item whose compared property is the upper bound of the query range
 * @return std::unordered_set of items within the specified range
 *
 * @note Returns an empty set if the end item is less than the start item
 * @example To select all Items with weights 0.4 to 10.9, we'd setup the class & parameters as such:
 *  - this Inventory object is of type Inventory<CompareItemWeight>
 *  - start = Item("some_name", 0.4, ItemType::None)
 *  - end = = Item("some_other_name", 10.9, ItemType::None)
 *
 */
template <class Comparator>
std::unordered_set<Item>
Inventory<Comparator, std::unordered_set<Item>>::query(const Item& start, const Item& end) const {
    std::unordered_set<Item> result;

    if (Comparator::lessThan(end, start)) return result;

    for (const auto& i : items_) {
        if (Comparator::leq(start, i) && Comparator::leq(i, end)) {
            result.insert(i);
        }
    }
    return result;
}

/**
 * Destructor
 */
template <class Comparator>
Inventory<Comparator, std::unordered_set<Item>>::~Inventory() {
    discardEquipped();
}
