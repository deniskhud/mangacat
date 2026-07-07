#include "renderer.hpp"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <iostream>
#include <ostream>
#include <string>
#include <unistd.h>

Renderer::Renderer::Renderer(const Cli::TermSize& termSize) : termSize(termSize) {
    enterAltScreen();
    hideCursor();
    renderBuffer = resizeRenderBuffer(termSize);
}

Renderer::Renderer::~Renderer() {
    showCursor();
    leaveAltScreen();
}

void Renderer::Renderer::resize(const Cli::TermSize& size) {
    termSize = size;
    renderBuffer = resizeRenderBuffer(termSize);
    lastImageWidth = 0;
    lastImageHeight = 0;
}

void Renderer::Renderer::beginFrame() {
    clearBuffer();
}

void Renderer::Renderer::endFrame() {
    render();
}

void Renderer::Renderer::render() {
    moveCursor(0, 0);
    drawFrame();
}

void Renderer::Renderer::setChar(std::uint32_t x, std::uint32_t y, char ch) {
    if (x < termSize.cols && y < termSize.rows) {
        renderBuffer[y][x] = ch;
    }
}

void Renderer::Renderer::clearBuffer() {
    for (auto& row : renderBuffer) {
        std::ranges::fill(row, ' ');
    }
}

void Renderer::Renderer::clearScreen() {
    writeRaw("\033[2J\033[H");
    lastImageWidth = 0;
    lastImageHeight = 0;
    loadingVisible = false;
}

std::vector<std::string> Renderer::Renderer::resizeRenderBuffer(const Cli::TermSize& size) {
    return std::vector<std::string>(size.rows, std::string(size.cols, ' '));
}

void Renderer::Renderer::drawFrame() {
    std::string output;
    output.reserve((termSize.cols + 16) * termSize.rows);

    for (std::uint32_t y = 0; y < termSize.rows; ++y) {
        const auto& row = renderBuffer[y];
        std::uint32_t x = 0;

        while (x < termSize.cols) {
            while (x < termSize.cols && row[x] == ' ') {
                ++x;
            }

            if (x >= termSize.cols) break;

            const std::uint32_t start = x;
            while (x < termSize.cols && row[x] != ' ') {
                ++x;
            }

            output += "\033[" + std::to_string(y + 1) + ";"
                + std::to_string(start + 1) + "H";
            output.append(row, start, x - start);
        }
    }

    writeRaw(output);
}

void Renderer::Renderer::drawImage(const std::shared_ptr<ImageData>& image) {
    if (!image || !image->loaded) {
        drawLoading();
        return;
    }
    if (loadingVisible) {
        clearRegion(loadingX, loadingY, loadingWidth, 1);
        loadingVisible = false;
    }
    std::cerr << "Image x " << image->layout.x
          << ", y " << image->layout.y
          << ". width : " << image->layout.layoutWidth
          << ", height: " << image->layout.layoutHeight << "\n" << std::flush; // Изменено здесь

    eraseImagesConflict(image->layout.x, image->layout.y, image->layout.layoutWidth, image->layout.layoutHeight);



    moveCursor(
        std::max(static_cast<std::uint32_t>(0), image->layout.x),
        std::max(static_cast<std::uint32_t>(0), image->layout.y)
    );

    for (const auto& chunk : image->chunks) {
        writeRaw(chunk.prefix);
        writeRaw(chunk.data);
        writeRaw("\033\\");
    }

    lastImageX = std::max(static_cast<std::uint32_t>(0), image->layout.x);
    lastImageY = std::max(static_cast<std::uint32_t>(0), image->layout.y);
    lastImageWidth = image->layout.layoutWidth;
    lastImageHeight = image->layout.layoutHeight;
}

void Renderer::Renderer::drawLoading() {
    if (!loadingVisible) {
        clearScreen();
    }

    const std::string_view text = "Loading...";
    const std::uint32_t x = termSize.cols > text.size()
        ? (termSize.cols - static_cast<std::uint32_t>(text.size())) / 2
        : 0;
    const std::uint32_t y = termSize.rows > 0 ? termSize.rows / 2 : 0;

    drawText(x, y, text);
    loadingVisible = true;
    loadingX = x;
    loadingY = y;
    loadingWidth = x < termSize.cols
        ? std::min<std::uint32_t>(static_cast<std::uint32_t>(text.size()), termSize.cols - x)
        : 0;
}

void Renderer::Renderer::drawStatus(std::size_t current, std::size_t total) {
    if (termSize.rows == 0) return;

    const std::string status = std::to_string(current) + " / " + std::to_string(total);
    clearRegion(0, termSize.rows - 1, termSize.cols, 1);
    drawText(0, termSize.rows - 1, status);
}

void Renderer::Renderer::drawText(std::uint32_t x, std::uint32_t y, std::string_view text) {
    if (y >= termSize.rows || x >= termSize.cols) return;

    const std::uint32_t maxLength = termSize.cols - x;
    const std::uint32_t length = std::min<std::uint32_t>(
        static_cast<std::uint32_t>(text.size()),
        maxLength
    );

    for (std::uint32_t i = 0; i < length; ++i) {
        setChar(x + i, y, text[i]);
    }
}

void Renderer::Renderer::drawRect(std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height) {
    if (width == 0 || height == 0) return;
    if (width == 1 && height == 1) {
        setChar(x, y, '+');
        return;
    }

    drawHLine(x, y, width, '-');
    if (height > 1) {
        drawHLine(x, y + height - 1, width, '-');
    }
    drawVLine(x, y, height, '|');
    if (width > 1) {
        drawVLine(x + width - 1, y, height, '|');
    }

    setChar(x, y, '+');
    if (width > 1) setChar(x + width - 1, y, '+');
    if (height > 1) setChar(x, y + height - 1, '+');
    if (width > 1 && height > 1) setChar(x + width - 1, y + height - 1, '+');
}

void Renderer::Renderer::drawHLine(std::uint32_t x, std::uint32_t y, std::uint32_t width, char ch) {
    for (std::uint32_t i = 0; i < width; ++i) {
        setChar(x + i, y, ch);
    }
}

void Renderer::Renderer::drawVLine(std::uint32_t x, std::uint32_t y, std::uint32_t height, char ch) {
    for (std::uint32_t i = 0; i < height; ++i) {
        setChar(x, y + i, ch);
    }
}

void Renderer::Renderer::clearRegion(std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height) {
    if (x >= termSize.cols || y >= termSize.rows || width == 0 || height == 0) return;

    const std::uint32_t boundedWidth = std::min(width, termSize.cols - x);
    const std::uint32_t boundedHeight = std::min(height, termSize.rows - y);
    const std::string spaces(boundedWidth, ' ');

    std::string output;
    output.reserve((spaces.size() + 16) * boundedHeight);

    for (std::uint32_t row = 0; row < boundedHeight; ++row) {
        output += "\033[" + std::to_string(y + row + 1) + ";"
            + std::to_string(x + 1) + "H";
        output += spaces;
    }

    writeRaw(output);
}

void Renderer::Renderer::moveCursor(std::uint32_t x, std::uint32_t y) {
    const std::string command = "\033[" + std::to_string(y + 1) + ";"
        + std::to_string(x + 1) + "H";
    writeRaw(command);
}

void Renderer::Renderer::writeRaw(std::string_view data) {
    while (!data.empty()) {
        ssize_t written = write(STDOUT_FILENO, data.data(), data.size());
        if (written > 0) {
            data.remove_prefix(static_cast<std::size_t>(written));
            continue;
        }
        if (written == -1 && errno == EINTR) {
            continue;
        }
        break;
    }
}

std::uint32_t Renderer::Renderer::imageCellWidth(const ImageLayout& layout) const {
    if (layout.layoutWidth <= 0) return 0;
    if (termSize.cols == 0 || termSize.pixelWidth == 0) return 0;

    const std::uint32_t cellWidth = termSize.pixelWidth / termSize.cols;
    if (cellWidth == 0) return 0;

    return (static_cast<std::uint32_t>(layout.layoutWidth) + cellWidth - 1) / cellWidth;
}

std::uint32_t Renderer::Renderer::imageCellHeight(const ImageLayout& layout) const {
    if (layout.layoutHeight <= 0) return 0;
    if (termSize.rows == 0 || termSize.pixelHeight == 0) return 0;

    const std::uint32_t cellHeight = termSize.pixelHeight / termSize.rows;
    if (cellHeight == 0) return 0;

    return (static_cast<std::uint32_t>(layout.layoutHeight) + cellHeight - 1) / cellHeight;
}

void Renderer::Renderer::enterAltScreen() {
    write(STDOUT_FILENO, "\033[?1049h", 8);
}

void Renderer::Renderer::leaveAltScreen() {
    write(STDOUT_FILENO, "\033[?1049l", 8);
}

void Renderer::Renderer::hideCursor() {
    write(STDOUT_FILENO, "\033[?25l", 6);
}

void Renderer::Renderer::showCursor() {
    write(STDOUT_FILENO, "\033[?25h", 6);
}

void Renderer::Renderer::eraseImagesConflict(const std::uint32_t& currentX, const std::uint32_t& currentY, const std::uint32_t& currentWidth, const std::uint32_t& currentHeight) {
    if ((lastImageHeight > currentHeight || lastImageWidth > currentWidth) ||
        (lastImageX < currentX || lastImageY > currentY)
    ) {
            writeRaw("\033_Ga=d,d=A,q=2\033\\");
    }
}