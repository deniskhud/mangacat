#include "renderer.hpp"

#include <iostream>
Renderer::Image::Image(const Cli::Terminal& term) : terminal(term){
    termSize = term.getTerminalSize();
    centerRow = (termSize.rows / 2) + 1;
    centerCol = (termSize.cols / 2) + 1;
}
void Renderer::Image::render(const std::shared_ptr<ImageData>& img) {
    if (!img || !img->loaded) {
        renderLoading();
        return;
    }
    if (needsClear(img->layout)) {
        terminal.clearAt(lastX, lastY, lastWidth, lastHeight);
    }

    setCursorCenter(img);

    for (const auto &c: img->chunks) {
        fwrite(c.prefix.data(), 1, c.prefix.size(), stdout);
        fwrite(c.data.data(), 1, c.data.size(), stdout);
        fwrite("\033\\", 1, 2, stdout);
    }
    fflush(stdout);
    lastWidth = img->layout.layoutWidth;
    lastHeight = img->layout.layoutHeight;
    lastX = img->layout.x;
    lastY = img->layout.y;
}

void Renderer::Image::renderLoading() {
    printf("\033[2J");
    printf("Loading...\n");
    fflush(stdout);
}

void Renderer::Renderer::render(const std::shared_ptr<ImageData>& data) {
    printf("\033[H");

    if (data && data->loaded) {
        imageEngine.render(data);
    } else {
        imageEngine.renderLoading();
    }

    // 3. Рисуем рамки из буфера ПОВЕРХ картинки
    // Чтобы не перерисовывать пустоту, проходим по буферу
    std::string output;
    output.reserve(termSize.rows * termSize.cols); // Оптимизация выделения

    for (unsigned int r = 0; r < termSize.rows; ++r) {
        //clearBuffer();
        // Перемещаем курсор на начало строки
        output += "\033[" + std::to_string(r + 1) + ";1H";

        for (unsigned int c = 0; c < termSize.cols; ++c) {
            char ch = renderBuffer[c][r];
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

void Renderer::Renderer::setChar(unsigned int x, unsigned int y, char ch) {
    if (x < termSize.cols && y < termSize.rows) {
        renderBuffer[x][y] = ch;
    }
}

// Полезно для рисования рамок
void Renderer::Renderer::drawRect(int x, int y, int w, int h) {
    for (int i = x; i < x + w; ++i) {
        setChar(i, y, '-');       // Верх
        setChar(i, y + h - 1, '-'); // Низ
    }
    for (int i = y; i < y + h; ++i) {
        setChar(x, i, '|');       // Лево
        setChar(x + w - 1, i, '|'); // Право
    }
}

void Renderer::Renderer::clearBuffer() {
    for (auto& col : renderBuffer) {
        std::fill(col.begin(), col.end(), ' '); // Заполняем пробелами (пустотой)
    }
}

void Renderer::Renderer::clearImageBuffer() {

}


void Renderer::Renderer::drawTopRightRect(int width, int height) {
    // Вычисляем X так, чтобы правый край рамки касался края терминала
    // -1, так как координаты начинаются с 0
    int x = termSize.cols - width - 1;
    int y = 2; // Небольшой отступ сверху

    drawRect(x, y, width, height);
}
