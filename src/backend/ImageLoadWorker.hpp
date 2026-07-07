#pragma once

#include <condition_variable>
#include <deque>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "ImageCache.hpp"
#include "ImageData.hpp"

namespace fs = std::filesystem;

namespace Backend {

class ImageLoadWorker {
public:
    using LoaderFn = std::function<std::shared_ptr<ImageData>(const fs::path&)>;

    enum class Priority {
        Normal,
        High,
    };

    ImageLoadWorker(ImageCache& cache, LoaderFn loader, size_t workerCount = defaultWorkerCount());
    ~ImageLoadWorker();

    ImageLoadWorker(const ImageLoadWorker&) = delete;
    ImageLoadWorker& operator=(const ImageLoadWorker&) = delete;

    static size_t defaultWorkerCount();

    void request(const fs::path& path, Priority priority = Priority::Normal);
    void stop();

private:
    void run();

    ImageCache& imageCache;
    LoaderFn loader;

    std::vector<std::thread> workers;
    std::mutex mutex;
    std::condition_variable wake;
    std::deque<fs::path> queue;
    std::unordered_set<std::string> queued;
    std::unordered_set<std::string> loading;
    bool stopping = false;
};

} // namespace Backend
