#include "viewerApp.hpp"
#include <stdexcept>

namespace App {

namespace {
constexpr int imageLoadPollMs = 50;
}

ViewerApp::ViewerApp(const std::string& path)
    : terminal()
    , scanner(path)
    , cache(24)
    , loader(terminal.getTerminalSize())
    , loadWorker(cache, loader)
    , navigator(scanner.getPages(), cache, loader, loadWorker)
    , renderer(terminal.getTerminalSize())
{
    if (scanner.isEmpty())
        throw std::runtime_error("No images found in: " + path);
}



void ViewerApp::run() {
    renderer.clearScreen();

    renderCurrent();

    while (running) {
        handleEvent(terminal.readEvent(waitingForCurrentImage ? imageLoadPollMs : -1));
    }

    renderer.clearScreen();
}



void ViewerApp::handleEvent(const Cli::InputEvent& event) {
    switch (event.type) {
        case Cli::InputEventType::Resize:
            handleResize();
            break;
        case Cli::InputEventType::Key:
            handleKey(event.key);
            break;
        case Cli::InputEventType::Timeout:
            if (waitingForCurrentImage) {
                renderCurrent();
            }
            break;
    }
}



void ViewerApp::handleKey(int key) {
    switch (key) {
        case 'q':
        case 'Q':
            running = false;
            break;

        case Cli::Key::arrowRight:
        case 'l':
        case ' ':
            if (navigator.hasNext()) {
                navigator.next();
                renderCurrent();
            }
            break;

        case Cli::Key::arrowLeft:
        case 'h':
            if (navigator.hasPrev()) {
                navigator.prev();
                renderCurrent();
            }
            break;

        default:
            break;
    }
}



void ViewerApp::handleResize() {
    terminal.refreshSize();
    renderer.resize(terminal.getTerminalSize());

    // TODO: пересчитать layout, когда renderer получит resize API.
    renderer.clearScreen();
    renderCurrent();
}

void ViewerApp::renderCurrent() {
    auto img = navigator.current();
    waitingForCurrentImage = !img || !img->loaded;
    renderer.beginFrame();
    renderer.drawImage(img);


    renderer.drawStatus(navigator.currentIndex() + 1, navigator.total());
    renderer.endFrame();
}

} // namespace App
