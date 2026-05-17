#ifndef RENDERER_HPP
#define RENDERER_HPP
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../backend/backend.hpp"
#include "../cli/cli.hpp"
#include "../backend/ImageData.hpp"

struct Pos {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;

    Pos() : x(0), y(0),  width(0), height(0) {  }
    Pos(uint16_t x, uint16_t y, uint16_t width, uint16_t height) :
        x(x), y(y),  width(width), height(height) {

    }
};

namespace Renderer {
    class Image {
    private:
        int last_w_ = 0;
        int last_h_  = 0;

        bool needs_clear(const ImageLayout& layout) const {
            // Первый кадр — всегда clear
            if (last_w_ == 0 && last_h_ == 0) return true;
            // Новая картинка меньше — старые пиксели останутся по краям
            return layout.out_w < last_w_ || layout.out_h < last_h_;
        }



        Pos position{};
        void set_cursor_center(const std::shared_ptr<ImageData>& img) {
            std::string move_cmd = "\033[" + std::to_string(img->layout.y) + ";"
                                   + std::to_string(img->layout.x) + "H";
            /*std::string move_cmd = "\033[" + std::to_string(center_row) + ";"
                                   + std::to_string(center_col) + "H";*/
            write(STDOUT_FILENO, move_cmd.c_str(), move_cmd.length());
        }
        Cli::TermSize termSize;
        Cli::Terminal term_;
        unsigned int center_col = 0, center_row = 0;
    public:
        Image(const Cli::Terminal& term);
        void render_loading();
        void render(const std::shared_ptr<ImageData>& img);
    };

    class Renderer {
    private:
        //buffer to render
        std::vector<std::vector<char>> render_buffer;
        std::vector<std::vector<std::string>> char_buffer;
        Cli::TermSize termSize;


        void set_char(unsigned int x, unsigned int y, char ch);
    public:
        void clear_buffer();
        void clear_image_buffer();
        void draw_top_right_rect(int width, int height);
        void draw_rect(int x, int y, int w, int h);
        Renderer(const Cli::TermSize& term_size) {
            termSize = term_size;
            render_buffer.resize(term_size.cols, std::vector<char>(term_size.rows));
        }
        void render(Image& img_engine, const std::shared_ptr<ImageData>& data);


    };
    struct RenderPos {
        int x, y;
    };

    class IRenderer {
    public:
        virtual ~IRenderer() = default;
        //virtual void show(const ImageData& img) = 0;
        virtual void clear() = 0;
    };




    inline void draw_separator(Cli::Terminal& term, const Cli::TermSize& size) {
        unsigned int left_width = size.cols / 2;
        for (unsigned int i = 1; i <= size.rows; ++i) {
            term.draw_at(left_width, i, "│");
        }
    }
    inline void draw_horizontal_separator(Cli::Terminal& term, const Cli::TermSize& size) {

    }


    struct ColumnLayout {
        unsigned int left_width;
        unsigned int right_width;

        ColumnLayout(const Cli::TermSize& size) {
            left_width = size.cols / 2;
            right_width = size.cols - left_width;
        }


    };
    class Bar {
    public:
        unsigned int x, y, width;

        Bar(unsigned int x, unsigned int y, unsigned int width) : x(x), y(y), width(width) {}

        void render(Cli::Terminal& term, float progress) {
            int filled = static_cast<int>(width * progress);
            std::string bar = "[";
            for (int i = 0; i < width; ++i) {
                bar += (i < filled) ? "#" : " ";
            }
            bar += "]";

            term.draw_at(x, y, bar);
        }
    };

}
#endif //RENDERER_HPP