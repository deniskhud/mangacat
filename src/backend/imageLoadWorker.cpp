#include "imageLoadWorker.hpp"

#include <algorithm>
#include <exception>
#include <utility>

namespace Backend {

ImageLoadWorker::ImageLoadWorker(ImageCache& cache, LoaderFn loaderParam, size_t workerCount)
    : imageCache(cache)
    , loader(std::move(loaderParam))
{
    workerCount = std::max<size_t>(1, workerCount);
    workers.reserve(workerCount);

    for (size_t i = 0; i < workerCount; ++i) {
        workers.emplace_back(&ImageLoadWorker::run, this);
    }
}

ImageLoadWorker::~ImageLoadWorker() {
    stop();
}

size_t ImageLoadWorker::defaultWorkerCount() {
    unsigned int hardware = std::thread::hardware_concurrency();
    if (hardware <= 2) return 1;
    return std::min<size_t>(4, hardware - 1);
}

void ImageLoadWorker::request(const fs::path& path, Priority priority) {
    if (imageCache.get(path)) return;

    bool shouldNotify = false;

    {
        std::lock_guard<std::mutex> lock(mutex);
        if (stopping) return;

        std::string key = path.string();
        if (loading.find(key) != loading.end()) return;

        if (queued.find(key) != queued.end()) {
            if (priority == Priority::High) {
                auto it = std::find_if(queue.begin(), queue.end(), [&key](const fs::path& queuedPath) {
                    return queuedPath.string() == key;
                });

                if (it != queue.end() && it != queue.begin()) {
                    fs::path pending = std::move(*it);
                    queue.erase(it);
                    queue.push_front(std::move(pending));
                    shouldNotify = true;
                }
            }
        } else {
            queued.insert(key);
            if (priority == Priority::High) {
                queue.push_front(path);
            } else {
                queue.push_back(path);
            }
            shouldNotify = true;
        }
    }

    if (shouldNotify) {
        wake.notify_one();
    }
}

void ImageLoadWorker::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (!stopping) {
            stopping = true;
            queue.clear();
            queued.clear();
        }
    }

    wake.notify_all();

    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ImageLoadWorker::run() {
    while (true) {
        fs::path path;
        std::string key;

        {
            std::unique_lock<std::mutex> lock(mutex);
            wake.wait(lock, [this] {
                return stopping || !queue.empty();
            });

            if (stopping) return;

            path = queue.front();
            queue.pop_front();
            key = path.string();
            queued.erase(key);
            loading.insert(key);
        }

        try {
            if (!imageCache.get(path)) {
                imageCache.getOrLoad(path, loader);
            }
        } catch (...) {
            // Keep the background loader alive; the foreground path can still report errors.
        }

        {
            std::lock_guard<std::mutex> lock(mutex);
            loading.erase(key);
        }
    }
}

} // namespace Backend
