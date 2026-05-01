#include "cli.hpp"

Cli::Terminal::Terminal() {
    write(STDOUT_FILENO, "\033[?1049h", 8);
    enable_raw_mode();
    //clear();
    refresh_size();
    hide_cursor();
}


void Cli::Terminal::enable_raw_mode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &old_termios);
    raw = old_termios;
    raw.c_oflag &= ~(OPOST);
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void Cli::Terminal::disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
}

void Cli::Terminal::refresh_size() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    size = { w.ws_col, w.ws_row, w.ws_ypixel, w.ws_xpixel };

}
