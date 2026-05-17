#include "ImageData.hpp"
#include "../cli/cli.hpp"
#include "../stb_image_resize2.h"
#include <algorithm>

ImageLayout ImageLayout::compute(const uint8_t*       pixels,
                                 int                  img_w,
                                 int                  img_h,
                                 const Cli::TermSize& term,
                                 int                  reserved_rows)
{
    ImageLayout layout;

    if (term.rows == 0 || term.cols == 0 ||
        term.pixel_width == 0 || term.pixel_height == 0)
        return layout;

    // Размер одной ячейки в пикселях
    const int cell_w = static_cast<int>(term.pixel_width)  / static_cast<int>(term.cols);
    const int cell_h = static_cast<int>(term.pixel_height) / static_cast<int>(term.rows);

    if (cell_w == 0 || cell_h == 0) return layout;

    // Доступная область с учётом зарезервированных строк UI
    const int avail_px_w = static_cast<int>(term.pixel_width);
    const int avail_px_h = static_cast<int>(term.pixel_height) - reserved_rows * cell_h;

    if (avail_px_w <= 0 || avail_px_h <= 0) return layout;

    // Масштаб с сохранением пропорций
    const float scale = std::min(
        static_cast<float>(avail_px_w) / img_w,
        static_cast<float>(avail_px_h) / img_h
    );

    layout.out_w = static_cast<int>(img_w * scale);
    layout.out_h = static_cast<int>(img_h * scale);

    layout.resized.resize(layout.out_w * layout.out_h * 3);
    stbir_resize_uint8_linear(
        pixels,           img_w,        img_h,        0,
        layout.resized.data(), layout.out_w, layout.out_h, 0,
        STBIR_RGB
    );

    // Центрирование в доступной области
    // +reserved_rows чтобы не заезжать под StatusBar
    const int img_cols = (layout.out_w + cell_w - 1) / cell_w;
    const int img_rows = (layout.out_h + cell_h - 1) / cell_h;

    const int avail_cols = static_cast<int>(term.cols);
    const int avail_rows = static_cast<int>(term.rows) - reserved_rows;

    layout.x = std::max(0, (avail_cols - img_cols) / 2);
    layout.y = std::max(0, (avail_rows - img_rows) / 2) + reserved_rows;

    return layout;
}