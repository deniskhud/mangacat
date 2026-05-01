#ifndef RENDERER_HPP
#define RENDERER_HPP
#include <cstdint>
#include <vector>
#include "../cli/cli.hpp"

struct Pos {
    uint16_t x;
    uint16_t y;
    Pos() : x(0), y(0) {  }
};

namespace Renderer {

    struct RenderPos {
        int x, y;
    };

    class IRenderer {
    public:
        virtual ~IRenderer() = default;
        //virtual void show(const ImageData& img) = 0;
        virtual void clear() = 0;
    };




    Cli::TermSize term_size;
    class Image {
    private:
        std::uint32_t width = 0, height = 0;
        Pos position;
        std::vector<std::uint8_t> pixels = {};
    public:
        Image();
        ~Image();
    };

    class Bar {
    private:

    public:

    };


}

#endif //RENDERER_HPP
