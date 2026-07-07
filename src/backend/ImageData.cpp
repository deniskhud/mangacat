#include "ImageData.hpp"
#include "../cli/cli.hpp"
#include "../stb_image_resize2.h"
#include <algorithm>

ImageLayout ImageLayout::computeImageLayout(const uint8_t* pixels, int imageWidth, int imageHeight, const Cli::TermSize& term, std::uint32_t reservedRows) {
    ImageLayout layout;

    if (term.rows == 0 || term.cols == 0 ||
        term.pixelWidth == 0 || term.pixelHeight == 0)
        return layout;

    // Размер одной ячейки в пикселях
    std::uint32_t cellWidth = term.pixelWidth  / term.cols;
    std::uint32_t cellHeight = term.pixelHeight / term.rows;

    if (cellWidth == 0 || cellHeight == 0) return layout;

    // Доступная область с учётом зарезервированных строк UI
    std::uint32_t availablePixelWidth = term.pixelWidth;
    std::uint32_t availablePixelHeight = term.pixelHeight - reservedRows * cellHeight;

    if (availablePixelWidth <= 0 || availablePixelHeight <= 0) return layout;

    // Масштаб с сохранением пропорций
    const float scale = std::min(
        static_cast<float>(availablePixelWidth) / imageWidth,
        static_cast<float>(availablePixelHeight) / imageHeight
    );

    layout.layoutWidth = static_cast<std::uint32_t>(imageWidth * scale);
    layout.layoutHeight = static_cast<std::uint32_t>(imageHeight * scale);

    layout.resized.resize(layout.layoutWidth * layout.layoutHeight * 3);
    stbir_resize_uint8_linear(
        pixels,           imageWidth,        imageHeight,        0,
        layout.resized.data(), layout.layoutWidth, layout.layoutHeight, 0,
        STBIR_RGB
    );

    // Центрирование в доступной области
    // +reserved_rows чтобы не заезжать под StatusBar
    std::uint32_t imageCols = (layout.layoutWidth + cellWidth - 1) / cellWidth;
    std::uint32_t imageRows = (layout.layoutHeight + cellHeight - 1) / cellHeight;
    layout.imageCols = imageCols;
    layout.imageRows = imageRows;

    std::uint32_t availableCols = term.cols;
    std::uint32_t availableRows = term.rows - reservedRows;

    layout.x = std::max(0, (static_cast<int>(availableCols) - static_cast<int>(imageCols)) / 2);
    layout.y = std::max(0, (static_cast<int>(availableRows) - static_cast<int>(imageRows)) / 2) + reservedRows;

    return layout;
}