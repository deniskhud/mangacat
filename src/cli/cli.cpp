#include "cli.hpp"


Cli::Terminal::Terminal() {
    size = query_size();
    center_row = (size.rows / 2) + 1;
    center_col = (size.cols / 2) + 1;
    write(STDOUT_FILENO, "\033[?1049h", 8);  // alternate screen
    enable_raw_mode();
    refresh_size();
    hide_cursor();
}

void Cli::Terminal::enable_raw_mode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &old_termios_);
    raw = old_termios_;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |=  (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}


int Cli::Terminal::read_key()const  {

    char c;
    while (read(STDIN_FILENO, &c, 1) != 1) {}

    if (c != '\033') return static_cast<int>(c);

    // Escape-последовательность: читаем остаток
    char seq[3] = {};
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return Key::ESC;
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return Key::ESC;

    if (seq[0] == '[') {
        // Стрелки: \033[A \033[B \033[C \033[D
        switch (seq[1]) {
            case 'A': return Key::ARROW_UP;
            case 'B': return Key::ARROW_DOWN;
            case 'C': return Key::ARROW_RIGHT;
            case 'D': return Key::ARROW_LEFT;
            case 'H': return Key::HOME;
            case 'F': return Key::END;
        }
        // Page Up/Down: \033[5~ \033[6~
        if (seq[1] == '5' || seq[1] == '6') {
            char tilde;
            read(STDIN_FILENO, &tilde, 1);
            if (tilde == '~')
                return seq[1] == '5' ? Key::PAGE_UP : Key::PAGE_DOWN;
        }
    }

    return Key::ESC;
}
/*
Cli::Terminal::Terminal() {
    size = get_terminal_size();
    center_row = (size.rows / 2) + 1;
    center_col = (size.cols / 2) + 1;
    write(STDOUT_FILENO, "\033[?1049h", 8);
    enable_raw_mode();
    //clear();
    refresh_size();
    set_cursor_center();
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
}*/