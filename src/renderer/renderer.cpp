#include "renderer.hpp"

#include <iostream>
Renderer::Image::Image(const Cli::Terminal& term) : term_(term){
    termSize = term.get_terminal_size();
    center_row = (termSize.rows / 2) + 1;
    center_col = (termSize.cols / 2) + 1;
}
void Renderer::Image::render(const std::shared_ptr<ImageData>& img) {
    if (!img || !img->loaded) {
        render_loading();
        return;// или нарисовать "Loading..."
    }
    if (needs_clear(img->layout)) {
        term_.clear_at(last_x_, last_y_, last_w_, last_h_);
    }

    set_cursor_center(img);

    for (const auto &c: img->chunks) {
        fwrite(c.prefix.data(), 1, c.prefix.size(), stdout);
        fwrite(c.data.data(), 1, c.data.size(), stdout);
        fwrite("\033\\", 1, 2, stdout);
    }
    fflush(stdout);
    last_w_ = img->layout.out_w;
    last_h_ = img->layout.out_h;
    last_x_ = img->layout.x;
    last_y_ = img->layout.y;
}

void Renderer::Image::render_loading() {
    printf("\033[2J");
    printf("Loading...\n");
    fflush(stdout);
}

void Renderer::Renderer::render(Image& img_engine, const std::shared_ptr<ImageData>& data) {
    printf("\033[H");

    // 2. Отрисовываем картинку
    // ВАЖНО: Внутри img_engine.render(data) уберите printf("\033[2J");
    if (data && data->loaded) {
        img_engine.render(data);
    } else {
        img_engine.render_loading();
    }

    // 3. Рисуем рамки из буфера ПОВЕРХ картинки
    // Чтобы не перерисовывать пустоту, проходим по буферу
    std::string output;
    output.reserve(termSize.rows * termSize.cols); // Оптимизация выделения

    for (unsigned int r = 0; r < termSize.rows; ++r) {
        //clear_buffer();
        // Перемещаем курсор на начало строки
        output += "\033[" + std::to_string(r + 1) + ";1H";

        for (unsigned int c = 0; c < termSize.cols; ++c) {
            char ch = render_buffer[c][r];
            if (ch != 0 && ch != ' ') { // Рисуем только значимые символы (рамки)
                output += ch;
            } else {
                // Если тут пусто, двигаем курсор вправо (не затирая картинку пробелом)
                output += "\033[C";
            }
        }
    }

    fwrite(output.c_str(), 1, output.length(), stdout);
    fflush(stdout);
}

void Renderer::Renderer::set_char(unsigned int x, unsigned int y, char ch) {
    if (x < termSize.cols && y < termSize.rows) {
        render_buffer[x][y] = ch;
    }
}

// Полезно для рисования рамок
void Renderer::Renderer::draw_rect(int x, int y, int w, int h) {
    for (int i = x; i < x + w; ++i) {
        set_char(i, y, '-');       // Верх
        set_char(i, y + h - 1, '-'); // Низ
    }
    for (int i = y; i < y + h; ++i) {
        set_char(x, i, '|');       // Лево
        set_char(x + w - 1, i, '|'); // Право
    }
}

void Renderer::Renderer::clear_buffer() {
    for (auto& col : render_buffer) {
        std::fill(col.begin(), col.end(), ' '); // Заполняем пробелами (пустотой)
    }
}

void Renderer::Renderer::clear_image_buffer() {

}


void Renderer::Renderer::draw_top_right_rect(int width, int height) {
    // Вычисляем X так, чтобы правый край рамки касался края терминала
    // -1, так как координаты начинаются с 0
    int x = termSize.cols - width - 1;
    int y = 2; // Небольшой отступ сверху

    draw_rect(x, y, width, height);
}
