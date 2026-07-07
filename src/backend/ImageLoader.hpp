#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include "ImageData.hpp"

namespace fs = std::filesystem;

namespace Backend {

    /**
     * ImageLoader — загружает одну картинку с диска в ImageData.
     *
     * Зависит только от stb_image и ImageData/ImageLayout.
     * Не знает ни о кеше, ни о навигации.
     *
     * Использование:
     *   ImageLoader loader;
     *   auto img = loader.load(path);   // nullptr если не удалось
     *
     *   // Или как callable для ImageCache::getOrLoad:
     *   cache.getOrLoad(path, loader);
     */
    class ImageLoader {
    public:
        // Главный метод. Возвращает nullptr при ошибке (файл не найден,
        // не поддерживаемый формат, и т.д.) — не бросает исключений.
        std::shared_ptr<ImageData> load(const fs::path& path) const;

        // operator() чтобы передавать как callable без лямбды
        std::shared_ptr<ImageData> operator()(const fs::path& path) const {
            return load(path);
        }

        ImageLoader(const Cli::TermSize& size) : termSize(size) {  }
    private:
        static std::string base64Encode(const uint8_t* data, size_t len);
        Cli::TermSize termSize;
        static constexpr const char* b64 =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    };

} // namespace Backend