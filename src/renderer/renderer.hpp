#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "../cli/cli.hpp"
#include "../backend/ImageData.hpp"

namespace Renderer {
    /** main render class
     * responsible for rendering the entire layer such as images, ui
     */
    class Renderer {
    public:
        explicit Renderer(const Cli::TermSize& termSize);
        ~Renderer();

        void resize(const Cli::TermSize& size);
        void beginFrame();
        void endFrame();
        void clearBuffer();
        void clearScreen();

        void drawImage(const std::shared_ptr<ImageData>& image);
        void drawLoading();
        void drawStatus(std::size_t current, std::size_t total);
        void drawText(std::uint32_t x, std::uint32_t y, std::string_view text);
        void drawRect(std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height);

    private:
        //buffer to render
        std::vector<std::string> renderBuffer;
        Cli::TermSize termSize;
        std::uint32_t lastImageX = 0;
        std::uint32_t lastImageY = 0;
        std::uint32_t lastImageWidth = 0;
        std::uint32_t lastImageHeight = 0;
        bool loadingVisible = false;
        std::uint32_t loadingX = 0;
        std::uint32_t loadingY = 0;
        std::uint32_t loadingWidth = 0;

        /** set char in (x, y) coordinate
         *
         * @param x
         * @param y
         * @param ch
         */
        void setChar(std::uint32_t x, std::uint32_t y, char ch);

        /** resize render buffer according to terminal size
         *
         * @param termSize
         * @return std::vector<std::vector<char>> buffer
         */
        std::vector<std::string> resizeRenderBuffer(const Cli::TermSize& size);

        void render();
        void drawFrame();
        void drawHLine(std::uint32_t x, std::uint32_t y, std::uint32_t width, char ch);
        void drawVLine(std::uint32_t x, std::uint32_t y, std::uint32_t height, char ch);
        void clearRegion(std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height);
        void moveCursor(std::uint32_t x, std::uint32_t y);
        void writeRaw(std::string_view data);
        [[nodiscard]] std::uint32_t imageCellWidth(const ImageLayout& layout) const;
        [[nodiscard]] std::uint32_t imageCellHeight(const ImageLayout& layout) const;

        static void enterAltScreen();
        static void leaveAltScreen();

        void eraseImagesConflict(const std::uint32_t& currentX, const std::uint32_t& currentY, const std::uint32_t& currentWidth, const std::uint32_t& currentHeight);

        /** hide cursor
         */
        static void hideCursor();
        static void showCursor();
    };
}
