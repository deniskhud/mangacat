#include <cstdio>
#include <iostream>
#include "cli/cli.hpp"
#include "renderer/image.hpp"
#include "backend/backend.hpp"

int main() {
    auto cli = Cli::init();
    auto back = Backend::DirectoryLoader("/home/khud/Загрузки/ReadBerserk_com_Berserk_v01-18/1/");
    auto image = Image("");

    image.render(back.get_image_data());
    std::cerr << "hello debug" << std::endl;
    while (true) {
        char c = getchar();
        if (c == 'q') {
            break;
        }
        if (c == 27) {
            if (getchar() == 91) {
                int arrow = getchar();
                if (arrow == 67) {
                    back.step_right();

                    image.render(back.get_image_data());
                }
                if (arrow == 68) {
                    back.step_left();
                    image.render(back.get_image_data());
                }
            }
        }
    }
    return 0;
}