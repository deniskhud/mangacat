#include "ViewerApp.hpp"
#include <iostream>

// ── Конструктор ────────────────────────────────────────────────────────────
//
// Порядок важен: scanner нужен до navigator, terminal до renderer.
// Member initializer list гарантирует порядок объявления в .hpp.

ViewerApp::ViewerApp(const std::string& path)
    : terminal_()
    , scanner_(path)
    , cache_(8)
    , loader_(terminal_.get_terminal_size())
    , navigator_(scanner_.get_pages(), cache_, loader_)
    , renderer_(terminal_)
{
    if (scanner_.is_empty())
        throw std::runtime_error("No images found in: " + path);
}

// ── Главный цикл ───────────────────────────────────────────────────────────

void ViewerApp::run() {
    //terminal_.set_raw_mode(true);
    terminal_.hide_cursor();
    terminal_.clear();

    render_current();

    while (running_) {
        int key = terminal_.read_key();
        handle_input(key);
    }

    terminal_.show_cursor();
    //terminal_._raw_mode(false);
    terminal_.clear();
}

// ── Ввод ───────────────────────────────────────────────────────────────────

void ViewerApp::handle_input(int key) {
    switch (key) {
        case 'q':
        case 'Q':
            running_ = false;
            break;

        case Cli::Key::ARROW_RIGHT:
        case 'l':
        case ' ':
            if (navigator_.has_next()) {
                navigator_.next();
                render_current();
            }
            break;

        case Cli::Key::ARROW_LEFT:
        case 'h':
            if (navigator_.has_prev()) {
                navigator_.prev();
                render_current();
            }
            break;

        default:
            break;
    }
}

// ── Рендер ─────────────────────────────────────────────────────────────────

void ViewerApp::render_current() {
    auto img = navigator_.current();
    renderer_.render(img);

    // Статус: "3 / 42  filename.jpg"
    auto size = terminal_.get_terminal_size();
    std::string status =
        std::to_string(navigator_.current_index() + 1) + " / " +
        std::to_string(navigator_.total());

    terminal_.draw_at(1, size.rows, status);
}