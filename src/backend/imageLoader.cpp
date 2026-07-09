#include "imageLoader.hpp"
#include "../stb_image.h"
#include <algorithm>
#include <iostream>

namespace Backend {

std::shared_ptr<ImageData> ImageLoader::load(const fs::path& path) const {
    int w, h, ch;
    uint8_t* pixels = stbi_load(path.c_str(), &w, &h, &ch, 3);

    if (!pixels) {
        std::cerr << "[ImageLoader] failed to load: " << path << "\n";
        return nullptr;
    }

    auto img    = std::make_shared<ImageData>();
    img->layout = ImageLayout::computeImageLayout(pixels, w, h, termSize);

    // Разбиваем base64 на chunks для kitty protocol
    std::string encoded = base64Encode(
        img->layout.resized.data(),
        img->layout.layoutWidth * img->layout.layoutHeight * 3
    );

    constexpr size_t chunkSizeLimit = 4096;
    const size_t total = encoded.size();

    for (size_t offset = 0; offset < total; offset += chunkSizeLimit) {
        size_t chunkSize = std::min(chunkSizeLimit, total - offset);
        bool isLast = offset + chunkSize >= total;
        int more = isLast ? 0 : 1;

        Chunk chunk;

        if (offset == 0) {
            chunk.prefix =
                "\033_Ga=T,f=24"
                ",s=" + std::to_string(img->layout.layoutWidth) +
                ",v=" + std::to_string(img->layout.layoutHeight) +
                ",x=0,y=0"
                ",C=1"
                ",m=" + std::to_string(more) + ";";
        } else {
            chunk.prefix = "\033_Gm=" + std::to_string(more) + ";";
        }

        chunk.data.assign(encoded.data() + offset, chunkSize);
        img->chunks.push_back(std::move(chunk));
    }

    stbi_image_free(pixels);
    img->loaded = true;

    //std::cerr << "[ImageLoader] loaded: " << path << "\n";
    return img;
}

std::string ImageLoader::base64Encode(const uint8_t* data, size_t len) {
    std::string out;
    out.reserve(((len + 2) / 3) * 4);

    for (size_t i = 0; i < len; i += 3) {
        uint32_t v = static_cast<uint32_t>(data[i]) << 16;
        if (i + 1 < len) v |= static_cast<uint32_t>(data[i + 1]) << 8;
        if (i + 2 < len) v |= static_cast<uint32_t>(data[i + 2]);

        out += b64[(v >> 18) & 63];
        out += b64[(v >> 12) & 63];
        out += (i + 1 < len) ? b64[(v >> 6) & 63] : '=';
        out += (i + 2 < len) ? b64[ v        & 63] : '=';
    }

    return out;
}

} // namespace Backend
