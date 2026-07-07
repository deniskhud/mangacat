#pragma once
#include <cstdint>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <atomic>
namespace Cli {

struct TermSize {
    std::uint32_t cols, rows, pixelHeight, pixelWidth;
};


namespace Key {
    constexpr int arrowLeft  = 1000;
    constexpr int arrowRight = 1001;
    constexpr int arrowUp    = 1002;
    constexpr int arrowDown  = 1003;
    constexpr int pageUp     = 1004;
    constexpr int pageDown   = 1005;
    constexpr int home       = 1006;
    constexpr int end        = 1007;
    constexpr int resize     = 1008;
    constexpr int esc        = 27;
    constexpr int tab        = 9;
}

enum class InputEventType {
    Key,
    Resize,
};

struct InputEvent {
    InputEventType type;
    int key = 0;

    static InputEvent keyEvent(int value) {
        return {InputEventType::Key, value};
    }

    static InputEvent resizeEvent() {
        return {InputEventType::Resize, 0};
    }
};

class Terminal final {
public:
    Terminal();

    ~Terminal() {
        showCursor();
        teardownSigwinch();
        disableRawMode();
        write(STDOUT_FILENO, "\033[?1049l", 8);
    }

    InputEvent readEvent() const;
    int readKey() const;

    // TODO cli не знает о draw
    void drawAt(unsigned int col, unsigned int row, const std::string& text) const {
        std::string cmd = "\033[" + std::to_string(row) + ";"
                        + std::to_string(col) + "H" + text;
        write(STDOUT_FILENO, cmd.c_str(), cmd.size());
    }
    /*completely clear the window */
    void clear()       { write(STDOUT_FILENO, "\033[2J\033[H", 8); }
    /*clear the window at certain coordinates */
    void clearAt(unsigned int col, unsigned int row, unsigned int width,  unsigned int height) const;
    void hideCursor() const { write(STDOUT_FILENO, "\033[?25l", 6); }
    void showCursor() const { write(STDOUT_FILENO, "\033[?25h", 6); }
    // TODO

    /* refresh size terminal */
    void refreshSize() {
        size = queryTermSize();
        centerRow = (size.rows / 2) + 1;
        centerCol = (size.cols / 2) + 1;
    }
    /* return current terminal size */
    [[nodiscard]] TermSize getTerminalSize() const { return size; }

private:
    /* requests the size of the terminal, for ex when changing the size */
    static TermSize queryTermSize() {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        return { w.ws_col, w.ws_row, w.ws_ypixel, w.ws_xpixel };
    }
    /* enable the raw terminal mode */
    void enableRawMode();

    /* disable the raw mode, in destructor */
    void disableRawMode() {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldTermios);
    }

    /* terminal size, pixel height/width */
    TermSize size{};
    std::uint32_t centerCol = 0;
    std::uint32_t centerRow = 0;
    /* default terminal settings */
    struct termios oldTermios{};


    int sigPipe[2] = {-1, -1};  // [0] = read end, [1] = write end
    static std::atomic<int> winchPipeWriteFd;  // для signal handler

    void setupSigwinch();
    void teardownSigwinch();
    static void sigwinchHandler(int);
    int readKeyFromStdin() const;
    bool readStdinByte(char& c, int timeoutMs) const;
    void drainResizePipe() const;
};
} // namespace Cli
