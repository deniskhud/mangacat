#pragma once
#include <filesystem>
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>
#include "ImageData.hpp"

namespace fs = std::filesystem;

namespace Backend {

class ImageCache {
public:
    explicit ImageCache(size_t max_size = 8) : max_size_(max_size) {}

    std::shared_ptr<ImageData> get(const fs::path& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        return get_unlocked(path.string());
    }

    void put(const fs::path& path, std::shared_ptr<ImageData> img) {
        std::lock_guard<std::mutex> lock(mutex_);
        put_unlocked(path.string(), std::move(img));
    }

    template<typename Loader>
    std::shared_ptr<ImageData> get_or_load(const fs::path& path, Loader&& loader) {
        // Сначала пробуем взять из кеша без загрузки
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (auto img = get_unlocked(path.string())) return img;
        }

        // Грузим без лока — загрузка долгая, не стоит держать мьютекс
        auto img = loader(path);

        // Кладём результат в кеш
        if (img) {
            std::lock_guard<std::mutex> lock(mutex_);
            put_unlocked(path.string(), img);
        }

        return img;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        map_.clear();
        lru_.clear();
    }

    size_t size() const { return map_.size(); }
    size_t capacity() const { return max_size_; }

private:
    struct Entry {
        std::shared_ptr<ImageData>       data;
        std::list<std::string>::iterator lru_it;
    };

    // Вызывать только под lock
    std::shared_ptr<ImageData> get_unlocked(const std::string& key) {
        auto it = map_.find(key);
        if (it == map_.end()) return nullptr;
        lru_.splice(lru_.begin(), lru_, it->second.lru_it);
        return it->second.data;
    }

    // Вызывать только под lock
    void put_unlocked(const std::string& key, std::shared_ptr<ImageData> img) {
        auto it = map_.find(key);
        if (it != map_.end()) {
            it->second.data = std::move(img);
            lru_.splice(lru_.begin(), lru_, it->second.lru_it);
            return;
        }
        if (map_.size() >= max_size_) evict();
        lru_.push_front(key);
        map_[key] = { std::move(img), lru_.begin() };
    }

    void evict() {
        if (lru_.empty()) return;
        map_.erase(lru_.back());
        lru_.pop_back();
    }

    mutable std::mutex mutex_;
    size_t max_size_;
    std::unordered_map<std::string, Entry> map_;
    std::list<std::string> lru_;
};

} // namespace Backend