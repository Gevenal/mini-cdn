#ifndef LRU_CACHE_HPP
#define LRU_CACHE_HPP

#include <list>          // For std::list (to maintain usage order)
#include <unordered_map> // For std::unordered_map (for O(1) average time lookups)
#include <optional>      // For std::optional (to return values from 'get' gracefully) C++17
#include <cstddef>       // For size_t
#include <stdexcept>     // If you plan to throw exceptions for errors like zero capacity

namespace Cache
{
    template <typename Key, typename Value>
    class LruCache
    {
    public:
        /**
         * @brief Constructs an LRU cache with a given capacity.
         * @param capacity The maximum number of items the cache can hold. Must be greater than 0.
         * @throw std::invalid_argument if capacity is 0 (optional error handling).
         */
        explicit LruCache(size_t capacity);

        /**
         * @brief Inserts or updates a key-value pair in the cache.
         * If the key already exists, its value is updated, and it's marked as recently used.
         * If the key doesn't exist and the cache is full, the least recently used item is evicted.
         * @param key The key of the item.
         * @param value The value of the item.
         */
        void put(const Key &key, const Value &value);

        /**
         * @brief Retrieves the value associated with a key.
         * If the key is found, it's marked as recently used.
         * @param key The key to look up.
         * @return An std::optional containing the value if the key is found; otherwise, std::nullopt.
         * (If not C++17, consider std::pair<Value, bool> or Value*).
         */
        std::optional<Value> get(const Key &key);

        /**
         * @brief Checks if the cache contains a specific key without updating its usage.
         * @param key The key to check for.
         * @return True if the key is present, false otherwise.
         */
        bool contains(const Key &key) const;

        /**
         * @brief Returns the current number of items in the cache.
         * @return The current size of the cache.
         */
        size_t size() const;

        /**
         * @brief Returns the maximum capacity of the cache.
         * @return The capacity of the cache.
         */
        size_t capacity() const;

        /**
         * @brief Removes all items from the cache.
         */
        void clear();

    private:
        struct CacheItem
        {
            Key key;
            Value value;
        };

        size_t capacity_;
        std::list<CacheItem> usage_list_;
        std::unordered_map<Key, typename std::list<CacheItem>::iterator> cache_map_;
    };

} // namespace Cache

#endif