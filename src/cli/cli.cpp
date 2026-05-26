#include "cli.hpp"

#include <csignal>
#include <poll.h>
#include <fcntl.h>

std::atomic<int> Cli::Terminal::winch_pipe_write_fd_{-1};

void Cli::Terminal::sigwinch_handler(int) {
    // Только async-signal-safe операции!
    int fd = winch_pipe_write_fd_.load(std::memory_order_relaxed);
    if (fd != -1) {
        char b = 1;
        ::write(fd, &b, 1);
    }
}

void Cli::Terminal::setup_sigwinch() {
    pipe2(sig_pipe_, O_NONBLOCK | O_CLOEXEC);
    winch_pipe_write_fd_.store(sig_pipe_[1], std::memory_order_relaxed);

    struct sigaction sa{};
    sa.sa_handler = sigwinch_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGWINCH, &sa, nullptr);
}

void Cli::Terminal::teardown_sigwinch() {
    signal(SIGWINCH, SIG_DFL);
    winch_pipe_write_fd_.store(-1, std::memory_order_relaxed);
    close(sig_pipe_[0]);
    close(sig_pipe_[1]);
}

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

void Cli::Terminal::clear_at(unsigned int col, unsigned int row, unsigned int width, unsigned int height) const {
    std::string spaces(width, ' ');
    for (size_t y = 0; y < height; ++y) {
        std::string cmd = "\033[" + std::to_string(row + y) + ";"
                        + std::to_string(col) + "H";

        write(STDOUT_FILENO, cmd.c_str(), cmd.size());
        write(STDOUT_FILENO, spaces.c_str(), spaces.size());
    }
}