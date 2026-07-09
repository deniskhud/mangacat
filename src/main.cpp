#include <iostream>
#include "app/viewerApp.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: viewer <directory>\n";
        return 1;
    }

    try {
        App::ViewerApp app(argv[1]);
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
