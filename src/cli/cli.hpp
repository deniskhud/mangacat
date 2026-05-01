#ifndef CLI_HPP
#define CLI_HPP
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

namespace Cli {

    struct TermSize {
        unsigned int cols, rows, pixel_height, pixel_width;
    };
    class Terminal {
    public:
        Terminal();

        ~Terminal() {
            disable_raw_mode();
            write(STDOUT_FILENO, "\033[?1049l", 8);
        }
    private:
        TermSize size{};
        struct termios old_termios{};

        /** enable raw terminal mode
         */
        void enable_raw_mode();
        void disable_raw_mode();

        void refresh_size();


        void hide_cursor() { write(STDOUT_FILENO, "\033[?25l", 6); }
        void show_cursor() { write(STDOUT_FILENO, "\033[?25h", 6); }
        void clear()       { write(STDOUT_FILENO, "\033[2J\033[H", 7); }

    };

    [[nodiscard]] inline TermSize get_terminal_size() {
        struct winsize ws;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
        return { ws.ws_col, ws.ws_row, ws.ws_ypixel, ws.ws_xpixel };
    }

    inline auto init() -> Terminal {
        return {};
    }
    class Bar {
    private:

    public:
    };
}

#endif //CLI_HPP
