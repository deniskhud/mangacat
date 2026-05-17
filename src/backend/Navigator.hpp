#pragma once
#include <deque>
#include <filesystem>
#include <functional>
#include <future>
#include <memory>
#include <vector>
#include "ImageCache.hpp"
#include "ImageData.hpp"

namespace fs = std::filesystem;

namespace Backend {

class Navigator {
public:
    static constexpr size_t LOOKAHEAD = 2;
    static constexpr size_t WINDOW    = 2 * LOOKAHEAD + 1;

    using LoaderFn = std::function<std::shared_ptr<ImageData>(const fs::path&)>;

    Navigator(const std::vector<fs::path>& pages, ImageCache& cache, LoaderFn loader)
        : pages_(pages), cache_(cache), loader_(std::move(loader))
    {
        build_window(0);
    }

    ~Navigator() {
        wait_prefetch();
    }

    // ── Навигация ─────────────────────────────────────────────────────

    void next() {
        if (!has_next()) return;
        ++cursor_;
        slide_right();
        prefetch_async(cursor_ + 1);   // грузим cursor+2 в фоне
    }

    void prev() {
        if (!has_prev()) return;
        --cursor_;
        slide_left();
        if (cursor_ >= 2)              // защита от size_t underflow
            prefetch_async(cursor_ - 1);
    }

    void jump(size_t index) {
        if (index >= pages_.size()) return;
        wait_prefetch();
        cursor_ = index;
        build_window(cursor_);
    }

    // ── Доступ ────────────────────────────────────────────────────────
    //
    // current() возвращает nullptr если картинка ещё не загружена —
    // рендер покажет Loading... и перерисует когда данные придут.

    std::shared_ptr<ImageData> current()      const { return window_get(cursor_); }
    const fs::path&            current_path() const { return pages_[cursor_]; }
    size_t                     current_index() const { return cursor_; }
    size_t                     total()         const { return pages_.size(); }
    bool                       has_next()      const { return cursor_ + 1 < pages_.size(); }
    bool                       has_prev()      const { return cursor_ > 0; }

private:
    // ── Окно ──────────────────────────────────────────────────────────

    std::deque<std::shared_ptr<ImageData>> deque_;
    size_t window_start_ = 0;

    void build_window(size_t index) {
        deque_.clear();
        size_t start     = (index >= LOOKAHEAD) ? index - LOOKAHEAD : 0;
        window_start_    = start;
        size_t end       = std::min(start + WINDOW, pages_.size());
        for (size_t i = start; i < end; ++i)
            deque_.push_back(load_sync(pages_[i]));
    }

    void slide_right() {
        if (cursor_ - window_start_ > LOOKAHEAD && !deque_.empty()) {
            deque_.pop_front();
            ++window_start_;
        }
        size_t right = window_start_ + deque_.size();
        if (right < pages_.size())
            deque_.push_back(load_sync(pages_[right]));
    }

    void slide_left() {
        size_t pos_in_window = cursor_ - window_start_;
        if (!deque_.empty() && (deque_.size() - 1 - pos_in_window) > LOOKAHEAD)
            deque_.pop_back();
        if (window_start_ > 0) {
            --window_start_;
            deque_.push_front(load_sync(pages_[window_start_]));
        }
    }

    std::shared_ptr<ImageData> window_get(size_t page_index) const {
        if (page_index < window_start_) return nullptr;
        size_t pos = page_index - window_start_;
        if (pos >= deque_.size()) return nullptr;
        return deque_[pos];
    }

    // ── Загрузка ──────────────────────────────────────────────────────

    // Синхронная — для текущего окна, зовётся из главного потока
    std::shared_ptr<ImageData> load_sync(const fs::path& p) {
        return cache_.get_or_load(p, loader_);
    }

    // Асинхронная предзагрузка — кладёт результат в кеш в фоне.
    // Следующий вызов load_sync для того же пути возьмёт из кеша мгновенно.
    std::future<void> prefetch_future_;

    void prefetch_async(size_t index) {
        if (index >= pages_.size()) return;
        if (cache_.get(pages_[index])) return;  // уже в кеше

        wait_prefetch();  // вариант А: ждём предыдущую задачу

        fs::path path = pages_[index];
        prefetch_future_ = std::async(std::launch::async, [this, path]() {
            cache_.get_or_load(path, loader_);
        });
    }

    void wait_prefetch() {
        if (prefetch_future_.valid())
            prefetch_future_.wait();
    }

    // ── Данные ────────────────────────────────────────────────────────

    const std::vector<fs::path>& pages_;
    ImageCache&                  cache_;
    LoaderFn                     loader_;
    size_t                       cursor_ = 0;
};

} // namespace Backend