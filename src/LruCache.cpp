#include "../include/proxy/LruCache.hpp"
#include "../include/proxy/HttpProxy.hpp"

namespace Cache
{
    template <typename Key, typename Value>
    LruCache<Key, Value>::LruCache(size_t capacity) : capacity_(capacity)
    {
        if (capacity == 0)
        {
            throw std::invalid_argument("LRU Cache Capacity must be greater than 0.");
        }
    }

    template <typename Key, typename Value>
    void LruCache<Key, Value>::put(const Key &key, const Value &value)
    {
        if (capacity_ == 0)
        {
            return;
        }

        auto it = cache_map_.find(key);
        // Key already exists in cache: Cache hit
        if (it != cache_map_.end())
        {
            it->second->value = value;
            usage_list_.splice(usage_list_.begin(), usage_list_, it->second);
        }
        else
        {
            if (usage_list_.size() >= capacity_)
            {
                if (!usage_list_.empty())
                {
                    const CacheItem &lru_item = usage_list_.back();
                    cache_map_.erase(lru_item.key);
                    usage_list_.pop_back(); // Remove from list
                }
            }
            // Add the new item to the front of the usage_list_ (MRU)
            usage_list_.emplace_front(CacheItem{key, value}); // Construct CacheItem in place

            // Add the new key and an iterator to its position in usage_list_ to the cache_map_
            cache_map_[key] = usage_list_.begin();
        }
    }

    template <typename Key, typename Value>
    bool LruCache<Key, Value>::contains(const Key &key) const
    {
        // if (capacity_ == 0) {
        //     return false;
        // }
        return cache_map_.count(key) > 0;
    }

    template <typename Key, typename Value>
    std::optional<Value> LruCache<Key, Value>::get(const Key &key)
    {
        if (capacity_ == 0)
        {
            return std::nullopt;
        }
        auto map_iterator = cache_map_.find(key);
        if (map_iterator == cache_map_.end())
        {
            return std::nullopt;
        }
        usage_list_.splice(usage_list_.begin(), usage_list_, map_iterator->second);
        return map_iterator->second->value;
    }

    template <typename Key, typename Value>
    size_t LruCache<Key, Value>::size() const
    {
        return usage_list_.size();
    }
    template <typename Key, typename Value>
    size_t LruCache<Key, Value>::capacity() const
    {

        return capacity_;
    }

    template <typename Key, typename Value>
    void LruCache<Key, Value>::clear()
    {
        usage_list_.clear();
        cache_map_.clear();
    }
}

template class Cache::LruCache<std::string, std::string>;
template class Cache::LruCache<int, int>;
template class Cache::LruCache<std::string, proxy::HttpProxy::ResponseCacheEntry>;