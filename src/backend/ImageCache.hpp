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
    explicit ImageCache(size_t maxSize = 8) : maxSize(maxSize) {}

    std::shared_ptr<ImageData> get(const fs::path& path) {
        std::lock_guard<std::mutex> lock(mutex);
        return getUnlocked(path.string());
    }

    void put(const fs::path& path, std::shared_ptr<ImageData> img) {
        std::lock_guard<std::mutex> lock(mutex);
        putUnlocked(path.string(), std::move(img));
    }

    template<typename Loader>
    std::shared_ptr<ImageData> getOrLoad(const fs::path& path, Loader&& loader) {
        // Сначала пробуем взять из кеша без загрузки
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (auto img = getUnlocked(path.string())) return img;
        }

        // Грузим без лока — загрузка долгая, не стоит держать мьютекс
        auto img = loader(path);

        // Кладём результат в кеш
        if (img) {
            std::lock_guard<std::mutex> lock(mutex);
            putUnlocked(path.string(), img);
        }

        return img;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex);
        entries.clear();
        lruKeys.clear();
    }

    size_t size() const { return entries.size(); }
    size_t capacity() const { return maxSize; }

private:
    struct Entry {
        std::shared_ptr<ImageData>       data;
        std::list<std::string>::iterator lruIt;
    };

    // Вызывать только под lock
    std::shared_ptr<ImageData> getUnlocked(const std::string& key) {
        auto it = entries.find(key);
        if (it == entries.end()) return nullptr;
        lruKeys.splice(lruKeys.begin(), lruKeys, it->second.lruIt);
        return it->second.data;
    }

    // Вызывать только под lock
    void putUnlocked(const std::string& key, std::shared_ptr<ImageData> img) {
        auto it = entries.find(key);
        if (it != entries.end()) {
            it->second.data = std::move(img);
            lruKeys.splice(lruKeys.begin(), lruKeys, it->second.lruIt);
            return;
        }
        if (entries.size() >= maxSize) evict();
        lruKeys.push_front(key);
        entries[key] = { std::move(img), lruKeys.begin() };
    }

    void evict() {
        if (lruKeys.empty()) return;
        entries.erase(lruKeys.back());
        lruKeys.pop_back();
    }

    mutable std::mutex mutex;
    size_t maxSize;
    std::unordered_map<std::string, Entry> entries;
    std::list<std::string> lruKeys;
};

} // namespace Backend