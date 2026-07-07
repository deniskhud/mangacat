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
    Timeout,
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

    static InputEvent timeoutEvent() {
        return {InputEventType::Timeout, 0};
    }
};

class Terminal final {
public:
    explicit Terminal();

    ~Terminal() {
        teardownSigwinch();
        disableRawMode();
    }

    InputEvent readEvent(int timeoutMs = -1) const;
    int readKey() const;

    /* refresh size terminal */
    void refreshSize();
    /* return current terminal size */
    [[nodiscard]] TermSize getTerminalSize() const { return size; }

private:
    /* requests the size of the terminal, for ex when changing the size */
    static TermSize queryTermSize();
    /* enable the raw terminal mode */
    void enableRawMode();

    /* disable the raw mode, in destructor */
    void disableRawMode();

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
