#ifndef VIEWERAPP_HPP
#define VIEWERAPP_HPP

#include <memory>
#include "../backend/DirectoryScanner.hpp"
#include "../backend/ImageCache.hpp"
#include "../backend/ImageLoader.hpp"
#include "../backend/Navigator.hpp"
#include "../cli/cli.hpp"
#include "../renderer/renderer.hpp"

/**
 * ViewerApp — единственная точка сборки.
 *
 * Владеет всеми компонентами и знает порядок их взаимодействия.
 * Остальные классы друг о друге не знают.
 *
 *   main.cpp:
 *     ViewerApp app("/path/to/dir");
 *     app.run();
 */
class ViewerApp {
public:
    explicit ViewerApp(const std::string& path);
    void run();

private:
    // ── Порядок объявления = порядок инициализации ─────────────────────
    Cli::Terminal            terminal_;
    Backend::DirectoryScanner scanner_;
    Backend::ImageCache       cache_;
    Backend::ImageLoader      loader_;
    Backend::Navigator        navigator_;
    Renderer::Image           renderer_;

    // ── Состояние ──────────────────────────────────────────────────────
    bool running_ = true;

    // ── Внутренние методы ──────────────────────────────────────────────
    void handle_input(int key);
    void render_current();
};


#endif //VIEWERAPP_HPP