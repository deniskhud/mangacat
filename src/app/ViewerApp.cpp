#include "ViewerApp.hpp"
#include <stdexcept>

namespace App {

ViewerApp::ViewerApp(const std::string& path)
    : terminal()
    , scanner(path)
    , cache(24)
    , loader(terminal.getTerminalSize())
    , loadWorker(cache, loader)
    , navigator(scanner.getPages(), cache, loader, loadWorker)
    , renderer(terminal)
{
    if (scanner.isEmpty())
        throw std::runtime_error("No images found in: " + path);
}



void ViewerApp::run() {
    terminal.clear();

    renderCurrent();

    while (running) {
        handleEvent(terminal.readEvent());
    }

    terminal.showCursor();
    terminal.clear();
}



void ViewerApp::handleEvent(const Cli::InputEvent& event) {
    switch (event.type) {
        case Cli::InputEventType::Resize:
            handleResize();
            break;
        case Cli::InputEventType::Key:
            handleKey(event.key);
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

    // TODO: пересчитать layout, когда renderer получит resize API.
    terminal.clear();
    renderCurrent();
}



void ViewerApp::renderCurrent() {
    auto img = navigator.current();
    renderer.render(img);


    auto size = terminal.getTerminalSize();
    std::string status =
        std::to_string(navigator.currentIndex() + 1) + " / " +
        std::to_string(navigator.total());

    terminal.drawAt(1, size.rows, status);
}

} // namespace App
