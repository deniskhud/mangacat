#include <iostream>
#include "backend/ViewerApp.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: viewer <directory>\n";
        return 1;
    }

    try {
        ViewerApp app(argv[1]);
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}


/*#include <cstdio>
#include <iostream>
#include "cli/cli.hpp"
#include "renderer/image.hpp"
#include "backend/backend.hpp"
#include "renderer/renderer.hpp"
int main() {
    auto cli = Cli::init();
    auto back = Backend::DirectoryLoader("/home/khud/Загрузки/ReadBerserk_com_Berserk_v01-18/1/");

    auto image = Renderer::Image(cli);
    auto render = Renderer::Renderer(cli.get_terminal_size());
    /*render.draw_top_right_rect(10, 10);
    render.render(image, back.get_image_data());#1#
    image.render(back.get_image_data());
    std::cerr << "hello debug" << std::endl;
    while (true) {
        char c = getchar();
        if (c == 'q') break;

        if (c == 27) {
            if (getchar() == 91) {
                int arrow = getchar();
                if (arrow == 67) back.step_right();
                if (arrow == 68) back.step_left();

                // 1. Очищаем старые символы в буфере (заполняем пробелами)
                //render.clear_buffer();

                // 2. Рисуем рамку в правом углу (например, 15 символов в ширину, 5 в высоту)
                //render.draw_top_right_rect(15, 5);
                image.render(back.get_image_data());
                // 3. Выводим всё на экран
                //render.render(image, back.get_image_data());
            }
        }
    }
    return 0;
}*/