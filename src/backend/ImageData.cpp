#include "ImageData.hpp"

#include "../stb_image_resize2.h"

ImageLayout ImageLayout::compute(const uint8_t* pixels, int img_w, int img_h) {
    ImageLayout layout;
    //TODO в будущем не будем вызывать каждый раз, только при изменении размера терминала
    auto term = Cli::get_terminal_size();
    // защита от деления на ноль
    if (term.rows == 0 || term.cols == 0 ||
        term.pixel_width == 0 || term.pixel_height == 0) {
        return layout;
        }
    unsigned img_pix_h = term.pixel_height - (term.pixel_height / term.rows);
    float scale = std::min(
        (float)term.pixel_width / img_w,
        (float)img_pix_h        / img_h
    );

    layout.out_w = (int)(img_w * scale);
    layout.out_h = (int)(img_h * scale);

    layout.resized.resize(layout.out_w * layout.out_h * 3);
    stbir_resize_uint8_linear(pixels, img_w, img_h, 0,
                               layout.resized.data(), layout.out_w, layout.out_h, 0, STBIR_RGB);

    int cell_w = term.pixel_width  / term.cols;
    int cell_h = term.pixel_height / term.rows;
    int img_cols = (layout.out_w + cell_w - 1) / cell_w;
    int img_rows = (layout.out_h + cell_h - 1) / cell_h;

    layout.x = std::max(0, static_cast<int>((term.cols - img_cols)) / 2);
    layout.y = std::max(0, static_cast<int>((term.rows - img_rows)) / 2);

    return layout;
}