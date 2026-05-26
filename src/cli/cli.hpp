#pragma once
#include <string>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <atomic>
namespace Cli {

struct TermSize {
    unsigned int cols, rows, pixel_height, pixel_width;
};

// Специальные клавиши — выше ASCII чтобы не пересекаться
namespace Key {
    constexpr int ARROW_LEFT  = 1000;
    constexpr int ARROW_RIGHT = 1001;
    constexpr int ARROW_UP    = 1002;
    constexpr int ARROW_DOWN  = 1003;
    constexpr int PAGE_UP     = 1004;
    constexpr int PAGE_DOWN   = 1005;
    constexpr int HOME        = 1006;
    constexpr int END         = 1007;
    constexpr int RESIZE = 1008;
    constexpr int ESC         = 27;
    constexpr int TAB         = 9;
}

class Terminal {
public:
    Terminal();

    ~Terminal() {
        show_cursor();
        disable_raw_mode();
        write(STDOUT_FILENO, "\033[?1049l", 8);  // restore screen
    }

    // ── Ввод ──────────────────────────────────────────────────────────
    // Блокирует до нажатия клавиши.
    // Возвращает ASCII-код или Cli::Key::* для спецклавиш.
    int read_key() const;

    // ── Вывод ─────────────────────────────────────────────────────────
    void draw_at(unsigned int col, unsigned int row, const std::string& text) const {
        std::string cmd = "\033[" + std::to_string(row) + ";"
                        + std::to_string(col) + "H" + text;
        write(STDOUT_FILENO, cmd.c_str(), cmd.size());
    }
    /*completely clear the window */
    void clear()       { write(STDOUT_FILENO, "\033[2J\033[H", 8); }
    /*clear the window at certain coordinates */
    void clear_at(unsigned int col, unsigned int row, unsigned int width,  unsigned int height) const;
    void hide_cursor() const { write(STDOUT_FILENO, "\033[?25l", 6); }
    void show_cursor() const { write(STDOUT_FILENO, "\033[?25h", 6); }

    /* refresh size terminal */
    void refresh_size() {
        size = query_size();
        center_row = (size.rows / 2) + 1;
        center_col = (size.cols / 2) + 1;
    }
    /* return current terminal size */
    [[nodiscard]] TermSize get_terminal_size() const { return size; }

private:
    /* requests the size of the terminal, for ex when changing the size */
    static TermSize query_size() {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        return { w.ws_col, w.ws_row, w.ws_ypixel, w.ws_xpixel };
    }
    /* enable the raw terminal mode */
    void enable_raw_mode();

    /* disable the raw mode, in destructor */
    void disable_raw_mode() {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_termios_);
    }

    /* terminal size, pixel height/width */
    TermSize size{};
    unsigned int center_col = 0;
    unsigned int center_row = 0;
    /* default terminal settings */
    struct termios old_termios_{};


    int sig_pipe_[2] = {-1, -1};  // [0] = read end, [1] = write end
    static std::atomic<int> winch_pipe_write_fd_;  // для signal handler

    void setup_sigwinch();
    void teardown_sigwinch();
    static void sigwinch_handler(int);
};
} // namespace Cli