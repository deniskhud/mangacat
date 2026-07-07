#include "ImageData.hpp"
#include "../cli/cli.hpp"
#include "../stb_image_resize2.h"
#include <algorithm>

ImageLayout ImageLayout::computeImageLayout(const uint8_t* pixels, int imageWidth, int imageHeight, const Cli::TermSize& term, int reservedRows) {
    ImageLayout layout;

    if (term.rows == 0 || term.cols == 0 ||
        term.pixelWidth == 0 || term.pixelHeight == 0)
        return layout;

    // Размер одной ячейки в пикселях
    const int cellWidth = static_cast<int>(term.pixelWidth)  / static_cast<int>(term.cols);
    const int cellHeight = static_cast<int>(term.pixelHeight) / static_cast<int>(term.rows);

    if (cellWidth == 0 || cellHeight == 0) return layout;

    // Доступная область с учётом зарезервированных строк UI
    const int availablePixelWidth = static_cast<int>(term.pixelWidth);
    const int availablePixelHeight = static_cast<int>(term.pixelHeight) - reservedRows * cellHeight;

    if (availablePixelWidth <= 0 || availablePixelHeight <= 0) return layout;

    // Масштаб с сохранением пропорций
    const float scale = std::min(
        static_cast<float>(availablePixelWidth) / imageWidth,
        static_cast<float>(availablePixelHeight) / imageHeight
    );

    layout.layoutWidth = static_cast<int>(imageWidth * scale);
    layout.layoutHeight = static_cast<int>(imageHeight * scale);

    layout.resized.resize(layout.layoutWidth * layout.layoutHeight * 3);
    stbir_resize_uint8_linear(
        pixels,           imageWidth,        imageHeight,        0,
        layout.resized.data(), layout.layoutWidth, layout.layoutHeight, 0,
        STBIR_RGB
    );

    // Центрирование в доступной области
    // +reserved_rows чтобы не заезжать под StatusBar
    const int imageCols = (layout.layoutWidth + cellWidth - 1) / cellWidth;
    const int imageRows = (layout.layoutHeight + cellHeight - 1) / cellHeight;

    const int availableCols = static_cast<int>(term.cols);
    const int availableRows = static_cast<int>(term.rows) - reservedRows;

    layout.x = std::max(0, (availableCols - imageCols) / 2);
    layout.y = std::max(0, (availableRows - imageRows) / 2) + reservedRows;

    return layout;
}