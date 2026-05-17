/*#include "backend.hpp"
#include <iostream>
#include "../stb_image.h"

Backend::DirectoryLoader::DirectoryLoader(const fs::path& cpath) {
    current_path = cpath;
    pages.reserve(100);
    get_images_from_directory();

}

void Backend::DirectoryLoader::change_directory(const fs::path &new_path) {
    current_path = new_path;
    pages.clear();
    get_images_from_directory();
    current_page = 0;
}

void Backend::DirectoryLoader::get_images_from_directory() {
    for (const auto& file : fs::directory_iterator(current_path)) {
        auto ext = file.path().extension().string();
        if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" ||
            ext == ".JPG" || ext == ".JPEG" || ext == ".PNG")
            pages.push_back(file.path());
    }
    // пока сортирую чисто для удобства отладки, в будущем будет плевать на название картинок
    std::sort(pages.begin(), pages.end(), [](const fs::path& a, const fs::path& b) {
        auto get_number = [](const fs::path& p) {
            std::string name = p.stem().string(); // "123" из "123.png"
            return std::stoi(name);
        };

        return get_number(a) < get_number(b);
    });

    //sort(pages.begin(), pages.end());
    deque_.clear();
    size_t count = std::min<size_t>(5, pages.size());

    for (size_t i = 0; i < count; ++i) {
        deque_.push_back(load_image(pages[i]));
    }

}

std::string Backend::DirectoryLoader::get_page_by_index(const size_t index) const {
    if (index >= pages.size()) return "";

    return pages[index].string();
}

void Backend::DirectoryLoader::step_right() {
    if (current_page + 1 >= pages.size()) return;
    move_right();

    if (current_page + 2 < pages.size()) {
        deque_.pop_front();
        deque_.push_back(load_image(pages[current_page + 2]));
    }
    debug_m();
}

void Backend::DirectoryLoader::step_left() {
    move_left();

    if (current_page >= 2) {
        deque_.pop_back();
        deque_.push_front(load_image(pages[current_page - 2]));
    }
    debug_m();
}

void Backend::DirectoryLoader::debug_m() {
    std::cerr << "[DEBUG] " << std::endl;
    std::cerr << "Current Page: " << current_page << std::endl;
    for (const auto& it : deque_) {
        std::cerr << it->path << std::endl;
    }
    std::cerr << "---------------------" << std::endl;

}



/*** image loader **#1#
Backend::ImageLoader::ImageLoader(const std::string_view &path) {

}

std::string Backend::ImageLoader::base64_encode(const uint8_t* data, size_t len) {
    std::string out;
    out.reserve(((len + 2) / 3) * 4);
    for (size_t i = 0; i < len; i += 3) {
        uint32_t v = data[i] << 16;
        if (i + 1 < len) v |= data[i+1] << 8;
        if (i + 2 < len) v |= data[i+2];
        out += b64[(v >> 18) & 63];
        out += b64[(v >> 12) & 63];
        out += (i + 1 < len) ? b64[(v >>  6) & 63] : '=';
        out += (i + 2 < len) ? b64[ v        & 63] : '=';
    }
    return out;
}

void Backend::ImageLoader::load_image(const fs::path& p) {
    auto img = std::make_shared<ImageData>();
    img->path = p.string();

    int w, h, ch;
    uint8_t* pixels = stbi_load(p.c_str(), &w, &h, &ch, 3);
    if (pixels) {
        img->layout = ImageLayout::compute(pixels, w, h);
        // если хочешь хранить сырые байты тоже:
        //img->pixels.assign(pixels, pixels + w * h * 3);

        std::string encoded = base64_encode(img->layout.resized.data(), img->layout.out_w * img->layout.out_h * 3);
        const size_t CHUNK = 4096;
        size_t total = encoded.size();
        size_t offset = 0;

        while (offset < total) {
            size_t chunk_size = std::min(CHUNK, total - offset);
            int more = (offset + chunk_size < total) ? 1 : 0;

            Chunk chunk;

            if (offset == 0) {
                //std::cerr << "w = " << img->layout.out_w << ", h = " << img->layout.out_h <<  std::endl;
                chunk.prefix = "\033_Ga=T,f=24,s=" + std::to_string(img->layout.out_w) +
               ",v=" + std::to_string(img->layout.out_h) +
               ",x=0,y=0," +
               "m=" + std::to_string(more) + ";";
            } else {
                chunk.prefix = "\033_Gm=" + std::to_string(more) + ";";
            }

            chunk.data.assign(encoded.data() + offset, chunk_size);

            img->chunks.push_back(std::move(chunk));

            offset += chunk_size;
        }

        stbi_image_free(pixels);
        img->loaded = true;
    }



    std::cerr << "Loaded image " << p.c_str() << std::endl;
    image = img;
}*/