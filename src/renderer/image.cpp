#include "image.hpp"

#include <iostream>

#include "../stb_image.h"
#include "../stb_image_resize2.h"

Image::Image(const std::string &path) : path(path) {

}

void Image::show() {
    int img_w, img_h, ch;
    uint8_t* pixels = stbi_load(path.c_str(), &img_w, &img_h, &ch, 3);
    if (!pixels) return;

    auto term = Cli::get_terminal_size();
    unsigned img_pix_h = term.pixel_height - (term.pixel_height / term.rows);

    float scale = std::min(
        (float)term.pixel_width  / img_w,
        (float)img_pix_h         / img_h
    );

    int out_w = (int)(img_w * scale);
    int out_h = (int)(img_h * scale);

    std::vector<uint8_t> resized(out_w * out_h * 3); //<---- размер около 2 миллионов
    stbir_resize_uint8_linear(pixels, img_w, img_h, 0,
                              resized.data(), out_w, out_h, 0, STBIR_RGB);
    stbi_image_free(pixels);

    int cell_w = term.pixel_width  / term.cols;
    int cell_h = term.pixel_height / term.rows;

    int img_cols = (out_w + cell_w - 1) / cell_w;
    int img_rows = (out_h + cell_h - 1) / cell_h;

    int x = std::max(0, static_cast<int>((term.cols - img_cols)) / 2);
    int y = std::max(0, static_cast<int>((term.rows - img_rows)) / 2);
    //std::cerr << resized.size() << std::endl;
    kitty_show(resized.data(), out_w, out_h, x, y);
}


void kitty_clear() {
    printf("\033_Ga=d,d=A\033\\");
}
//render Image, x, y и все
void kitty_show(const uint8_t* pixels, int w, int h, int x, int y) {
    //kitty_clear();
    std::string encoded = base64_encode(pixels, w * h * 3);
    //std::cerr << encoded.size() << std::endl;
    const size_t CHUNK = 4096;
    size_t total  = encoded.size();
    size_t offset = 0;
    printf("\033[H\033[2J");
    while (offset < total) {
        size_t chunk_size = std::min(CHUNK, total - offset);
        std::string chunk = encoded.substr(offset, chunk_size);
        int more = (offset + chunk_size < total) ? 1 : 0;

        if (offset == 0)
            printf("\033_Ga=T,f=24,s=%d,v=%d,x=%d,y=%d,m=%d;%s\033\\",
       w, h, x, y, more, chunk.c_str());

        else
            printf("\033_Gm=%d;%s\033\\", more, chunk.c_str());

        offset += chunk_size;
    }

    fflush(stdout);
}

std::string base64_encode(const uint8_t* data, size_t len) {
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

void Image::render(const ImageData& img) {
    printf("\033[H\033[2J");

    for (const auto& c : img.chunks) {
        fwrite(c.prefix.data(), 1, c.prefix.size(), stdout);
        fwrite(c.data.data(), 1, c.data.size(), stdout);
        fwrite("\033\\", 1, 2, stdout);
    }

    fflush(stdout);
    /*if (!img.loaded) return;
    kitty_show(img.layout.resized.data(), img.layout.out_w, img.layout.out_h, img.layout.x, img.layout.y);*/
}