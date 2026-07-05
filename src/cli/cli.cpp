#include "cli.hpp"

#include <cerrno>
#include <csignal>
#include <poll.h>
#include <fcntl.h>

std::atomic<int> Cli::Terminal::winch_pipe_write_fd_{-1};


Cli::Terminal::Terminal() {
    write(STDOUT_FILENO, "\033[?1049h", 8);

    enable_raw_mode();
    setup_sigwinch();
    refresh_size();
    hide_cursor();
}

void Cli::Terminal::sigwinch_handler(int) {
    // Только async-signal-safe операции!
    int fd = winch_pipe_write_fd_.load(std::memory_order_relaxed);
    if (fd != -1) {
        char b = 1;
        ::write(fd, &b, 1);
    }
}

void Cli::Terminal::setup_sigwinch() {
    if (pipe2(sig_pipe_, O_NONBLOCK | O_CLOEXEC) == -1) return;

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
    if (sig_pipe_[0] != -1) close(sig_pipe_[0]);
    if (sig_pipe_[1] != -1) close(sig_pipe_[1]);
    sig_pipe_[0] = -1;
    sig_pipe_[1] = -1;
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

Cli::InputEvent Cli::Terminal::read_event() const {
    while (true) {
        pollfd fds[2] = {
            {STDIN_FILENO, POLLIN, 0},
            {sig_pipe_[0], POLLIN, 0},
        };
        const nfds_t fd_count = sig_pipe_[0] == -1 ? 1 : 2;

        int result = poll(fds, fd_count, -1);
        if (result == -1) {
            if (errno == EINTR) continue;
            return InputEvent::key_event(Key::ESC);
        }

        if (fd_count == 2 && (fds[1].revents & POLLIN)) {
            drain_resize_pipe();
            return InputEvent::resize_event();
        }

        if (fds[0].revents & POLLIN) {
            return InputEvent::key_event(read_key_from_stdin());
        }
    }
}


int Cli::Terminal::read_key() const {
    InputEvent event = read_event();
    if (event.type == InputEventType::Resize) return Key::RESIZE;
    return event.key;
}

int Cli::Terminal::read_key_from_stdin() const {
    char c;
    if (!read_stdin_byte(c, -1)) return Key::ESC;

    if (c != '\033') return static_cast<int>(c);

    // Escape-последовательность: читаем остаток
    char seq[3] = {};
    if (!read_stdin_byte(seq[0], 25)) return Key::ESC;
    if (!read_stdin_byte(seq[1], 25)) return Key::ESC;

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
            if (!read_stdin_byte(tilde, 25)) return Key::ESC;
            if (tilde == '~')
                return seq[1] == '5' ? Key::PAGE_UP : Key::PAGE_DOWN;
        }
    }

    return Key::ESC;
}

bool Cli::Terminal::read_stdin_byte(char& c, int timeout_ms) const {
    while (true) {
        pollfd fd = {STDIN_FILENO, POLLIN, 0};
        int result = poll(&fd, 1, timeout_ms);

        if (result == 0) return false;
        if (result == -1) {
            if (errno == EINTR) continue;
            return false;
        }
        if (fd.revents & (POLLERR | POLLHUP | POLLNVAL)) return false;
        if (!(fd.revents & POLLIN)) continue;

        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n == 1) return true;
        if (n == -1 && errno == EINTR) continue;
        return false;
    }
}

void Cli::Terminal::drain_resize_pipe() const {
    if (sig_pipe_[0] == -1) return;

    char buf[64];
    while (read(sig_pipe_[0], buf, sizeof(buf)) > 0) {}
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
