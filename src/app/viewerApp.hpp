#pragma once

#include <string>
#include "../backend/directoryScanner.hpp"
#include "../backend/imageCache.hpp"
#include "../backend/imageLoadWorker.hpp"
#include "../backend/imageLoader.hpp"
#include "../backend/navigator.hpp"
#include "../cli/cli.hpp"
#include "../renderer/renderer.hpp"

namespace App {

/**
 * ViewerApp — application composition root.
 *
 * Владеет CLI, backend и renderer-компонентами и знает порядок их
 * взаимодействия. Поэтому живёт в app-слое, а не в backend.
 *
 *   main.cpp:
 *     App::ViewerApp app("/path/to/dir");
 *     app.run();
 */
class ViewerApp {
public:
    explicit ViewerApp(const std::string& path);
    void run();

private:

    Cli::Terminal terminal;
    Backend::DirectoryScanner scanner;
    Backend::ImageCache cache;
    Backend::ImageLoader loader;
    Backend::ImageLoadWorker loadWorker;
    Backend::Navigator navigator;
    Renderer::Renderer renderer;

    bool running = true;
    bool waitingForCurrentImage = false;

    void handleEvent(const Cli::InputEvent& event);
    void handleKey(int key);
    void handleResize();
    void renderCurrent();
};

} // namespace App
