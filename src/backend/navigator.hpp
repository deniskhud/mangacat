#pragma once
#include <algorithm>
#include <deque>
#include <filesystem>
#include <functional>
#include <memory>
#include <utility>
#include <vector>
#include "imageCache.hpp"
#include "imageData.hpp"
#include "imageLoadWorker.hpp"

namespace fs = std::filesystem;

namespace Backend {

class Navigator {
public:
    static constexpr size_t lookahead = 2;
    static constexpr size_t windowSize = 2 * lookahead + 1;
    static constexpr size_t forwardPrefetch = 8;
    static constexpr size_t backwardPrefetch = 3;

    using LoaderFn = std::function<std::shared_ptr<ImageData>(const fs::path&)>;

    Navigator(
        const std::vector<fs::path>& pages,
        ImageCache& cache,
        LoaderFn loader,
        ImageLoadWorker& loadWorkerRef
    )
        : pages(pages)
        , cache(cache)
        , loader(std::move(loader))
        , loadWorker(loadWorkerRef)
    {
        buildWindow(0);
    }

    // ── Навигация ─────────────────────────────────────────────────────

    void next() {
        if (!hasNext()) return;
        direction = Direction::Forward;
        ++cursor;
        slideRight();
        prefetchNearby();
    }

    void prev() {
        if (!hasPrev()) return;
        direction = Direction::Backward;
        --cursor;
        slideLeft();
        prefetchNearby();
    }

    void jump(size_t index) {
        if (index >= pages.size()) return;
        cursor = index;
        buildWindow(cursor);
    }

    // ── Доступ ────────────────────────────────────────────────────────
    //
    // Текущая страница берётся из окна или кеша. Если worker не успел,
    // renderer может показать Loading без блокировки главного потока.

    std::shared_ptr<ImageData> current()      { return windowGet(cursor); }
    const fs::path&            currentPath() const { return pages[cursor]; }
    size_t                     currentIndex() const { return cursor; }
    size_t                     total()         const { return pages.size(); }
    bool                       hasNext()      const { return cursor + 1 < pages.size(); }
    bool                       hasPrev()      const { return cursor > 0; }

private:
    // ── Окно ──────────────────────────────────────────────────────────

    std::deque<std::shared_ptr<ImageData>> deque;
    size_t windowStart = 0;
    enum class Direction {
        Forward,
        Backward,
    };
    Direction direction = Direction::Forward;

    void buildWindow(size_t index) {
        deque.clear();
        size_t start = (index >= lookahead) ? index - lookahead : 0;
        windowStart = start;
        size_t end = std::min(start + windowSize, pages.size());
        for (size_t i = start; i < end; ++i) {
            if (i == cursor) {
                deque.push_back(loadSync(pages[i]));
            } else {
                deque.push_back(cachedOrRequest(i, ImageLoadWorker::Priority::Normal));
            }
        }
        prefetchNearby();
    }

    void slideRight() {
        if (cursor - windowStart > lookahead && !deque.empty()) {
            deque.pop_front();
            ++windowStart;
        }
        size_t right = windowStart + deque.size();
        if (right < pages.size())
            deque.push_back(cachedOrRequest(right, ImageLoadWorker::Priority::High));
    }

    void slideLeft() {
        size_t posInWindow = cursor - windowStart;
        if (!deque.empty() && (deque.size() - 1 - posInWindow) > lookahead)
            deque.pop_back();
        if (windowStart > 0) {
            --windowStart;
            deque.push_front(cachedOrRequest(windowStart, ImageLoadWorker::Priority::High));
        }
    }

    std::shared_ptr<ImageData> windowGet(size_t pageIndex) {
        if (pageIndex < windowStart) return nullptr;
        size_t pos = pageIndex - windowStart;
        if (pos >= deque.size()) return nullptr;

        auto& slot = deque[pos];
        if (!slot) {
            slot = cache.get(pages[pageIndex]);
            if (!slot) {
                loadWorker.request(pages[pageIndex], ImageLoadWorker::Priority::High);
            }
        }

        return slot;
    }

    // ── Загрузка ──────────────────────────────────────────────────────

    // Синхронная — для текущего окна, зовётся из главного потока
    std::shared_ptr<ImageData> loadSync(const fs::path& p) {
        return cache.getOrLoad(p, loader);
    }

    std::shared_ptr<ImageData> cachedOrRequest(size_t index, ImageLoadWorker::Priority priority) {
        if (index >= pages.size()) return nullptr;

        auto img = cache.get(pages[index]);
        if (!img) {
            loadWorker.request(pages[index], priority);
        }
        return img;
    }

    void prefetchIndex(size_t index, ImageLoadWorker::Priority priority) {
        if (index >= pages.size()) return;
        loadWorker.request(pages[index], priority);
    }

    void prefetchNearby() {
        if (direction == Direction::Forward) {
            prefetchForward(ImageLoadWorker::Priority::High);
            prefetchBackward(ImageLoadWorker::Priority::Normal);
        } else {
            prefetchBackward(ImageLoadWorker::Priority::High);
            prefetchForward(ImageLoadWorker::Priority::Normal);
        }
    }

    void prefetchForward(ImageLoadWorker::Priority priority) {
        size_t first = cursor + 1;
        size_t last = std::min(cursor + forwardPrefetch, pages.size() - 1);

        if (first > last) return;

        if (priority == ImageLoadWorker::Priority::High) {
            for (size_t index = last + 1; index-- > first;) {
                prefetchIndex(index, priority);
            }
        } else {
            for (size_t index = first; index <= last; ++index) {
                prefetchIndex(index, priority);
            }
        }
    }

    void prefetchBackward(ImageLoadWorker::Priority priority) {
        if (cursor == 0) return;

        size_t first = cursor > backwardPrefetch ? cursor - backwardPrefetch : 0;
        size_t last = cursor - 1;

        if (priority == ImageLoadWorker::Priority::High) {
            for (size_t index = first; index <= last; ++index) {
                prefetchIndex(index, priority);
            }
        } else {
            for (size_t index = last + 1; index-- > first;) {
                prefetchIndex(index, priority);
            }
        }
    }
    const std::vector<fs::path>& pages;
    ImageCache&                  cache;
    LoaderFn                     loader;
    ImageLoadWorker&             loadWorker;
    size_t                       cursor = 0;
};

} // namespace Backend
