#include "TreeInventory.hpp"

/**
 * @brief Default constructor for the Inventory template class.
 *
 * Initializes an empty inventory with no items, no equipped item,
 * and zero total weight.
 *
 * @tparam Comparator The comparison class for querying items
 */
template <class Comparator>
Inventory<Comparator, Tree>::Inventory()
    : items_ { ItemAVL<Comparator>() }
    , equipped_ { nullptr }
    , weight_ { 0.0 }
{
}

/**
 * @brief Retrieves the value stored in `equipped_`
 * @return The Item pointer stored in `equipped_`
 */
template <class Comparator>
Item* Inventory<Comparator, Tree>::getEquipped() const
{
    return equipped_;
}

/**
 * @brief Equips a new item.
 * @param itemToEquip A pointer to the item to equip.
 * @post Updates `equipped` to the specified item
 * without deallocating the original.
 */
template <class Comparator>
void Inventory<Comparator, Tree>::equip(Item* itemToEquip)
{
    equipped_ = itemToEquip;
}

/**
 * @brief Discards the currently equipped item.
 * @post Deallocates the item pointed to by `equipped`
 * and sets `equipped` to nullptr, if `equipped` is not nullptr already.
 */
template <class Comparator>
void Inventory<Comparator, Tree>::discardEquipped()
{
    if (!equipped_) {
        return;
    }
    delete equipped_;
    equipped_ = nullptr;
}

/**
 * @brief Retrieves the value stored in `weight_`
 * @return The float value stored in `weight_`
 */
template <class Comparator>
float Inventory<Comparator, Tree>::getWeight() const
{
    return weight_;
}

/**
 * @brief Retrieves the count of Items in the items_ ItemAVL
 */
template <class Comparator>
size_t Inventory<Comparator, Tree>::size() const
{
    return items_.size();
}

/**
 * @brief Retrieves a copy of the container holding inventory items.
 * @return An ItemAVL with the correct Comparison class in the inventory
 */
template <class Comparator>
ItemAVL<Comparator> Inventory<Comparator, Tree>::getItems() const
{
    return items_;
}

/**
 * @brief Attempts to add a new item to the inventory.
 *
 * @param target Item to be added to the inventory
 * @return true if the item was successfully added, false if an item
 *         with the same name already exists
 */
template <class Comparator>
bool Inventory<Comparator, Tree>::pickup(const Item& target)
{
    if (items_.insert(target)) {
        weight_ += target.weight_;
        return true;
    }
    return false;
}

/**
 * @brief Attempts to remove an item from the inventory by name.
 *
 * @param name Name of the item to be removed
 * @return true if the item was successfully removed, false if the
 *         item was not found in the inventory
 */
template <class Comparator>
bool Inventory<Comparator, Tree>::discard(const std::string& itemName)
{
    float erased_weight = items_.erase(itemName);
    if (erased_weight) {
        weight_ -= erased_weight;
        return true;
    }
    return false;
}

/**
 * @brief Checks if an item with the given name exists in the inventory.
 *
 * @param name Name of the item to search for
 * @return true if the item exists in the inventory, false otherwise
 */
template <class Comparator>
bool Inventory<Comparator, Tree>::contains(const std::string& itemName) const
{
    return items_.contains(itemName);
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
std::unordered_set<Item> Inventory<Comparator, Tree>::query(
    const Item& start,
    const Item& end) const
{
    std::unordered_set<Item> result;

    // Invalid range → empty
    if (Comparator::lessThan(end, start)) {
        return result;
    }

    queryHelper(start, end, items_.root(), result);
    return result;
}

/**
 * @brief Recursive helper function for performing a range query on the AVL tree.
 *
 * This function is the core of the tree-based query operation. Instead of
 * scanning every item (like in the hash version), it takes advantage of the
 * AVL tree’s ordering to skip entire subtrees that cannot contain valid results.
 *
 * The goal is to collect all items such that:
 *      start <= item <= end   (inclusive range)
 *
 * @param start The lower bound Item (based on Comparator)
 * @param end The upper bound Item (based on Comparator)
 * @param root The current node being examined
 * @param result The set where valid items are inserted
 *
 * LOGIC BREAKDOWN:
 *
 * 1. BASE CASE:
 *    If root is nullptr, we reached the end of a branch → nothing to process.
 *
 * 2. LEFT SUBTREE:
 *    We ONLY go left if start <= current.
 *
 *    Why?
 *    - If current < start, then everything in the left subtree is even smaller.
 *    - That means nothing on the left can be in the valid range → skip it.
 *    - So we only recurse left when there is still a chance to find valid items.
 *
 * 3. CURRENT NODE:
 *    We check if the current node is within the range:
 *          start <= current <= end
 *
 *    If true, we insert it into the result set.
 *    We use leq (<=) to make the range inclusive, which is required by the spec.
 *
 * 4. RIGHT SUBTREE:
 *    We ONLY go right if current <= end.
 *
 *    Why?
 *    - If current > end, then everything in the right subtree is even larger.
 *    - That means nothing on the right can be valid → skip it.
 *
 * OVERALL IDEA:
 * Instead of visiting all nodes (O(n)), we prune branches early,
 * making the query more efficient (closer to O(log n) in balanced cases).
 */
template <class Comparator>
void Inventory<Comparator, Tree>::queryHelper(
    const Item& start, const Item& end, const Node* root, std::unordered_set<Item>& result) const 
{
    if (!root) return;

    const Item& current = root->value_;

    // Only go left if 'start' is less than the current node.
    // If start >= current, nothing in the left subtree can be in range.
    if (Comparator::leq(start, current)) {
        queryHelper(start, end, root->left_, result);
    }
    
    // Add current item if it is within [start, end]
    if (Comparator::leq(start, current) && Comparator::leq(current, end)) {
        result.insert(current);
    }

    // Only go right if 'end' is greater than the current node.
    // If end <= current, nothing in the right subtree can be in range.
    if (Comparator::leq(current, end)) {
        queryHelper(start, end, root->right_, result);
    }
    
}

/**
 * @brief Destructor for the Inventory class.
 * @post Deallocates any dynamically allocated resources.
 */
template <class Comparator>
Inventory<Comparator, Tree>::~Inventory() { discardEquipped(); }
