#include "cli.hpp"

#include <cerrno>
#include <csignal>
#include <poll.h>
#include <fcntl.h>

std::atomic<int> Cli::Terminal::winchPipeWriteFd{-1};


Cli::Terminal::Terminal() {
    enableRawMode();
    setupSigwinch();
    refreshSize();
}

void Cli::Terminal::sigwinchHandler(int) {
    // Только async-signal-safe операции!
    int fd = winchPipeWriteFd.load(std::memory_order_relaxed);
    if (fd != -1) {
        char b = 1;
        ::write(fd, &b, 1);
    }
}

void Cli::Terminal::setupSigwinch() {
    if (pipe2(sigPipe, O_NONBLOCK | O_CLOEXEC) == -1) return;

    winchPipeWriteFd.store(sigPipe[1], std::memory_order_relaxed);

    struct sigaction sa{};
    sa.sa_handler = sigwinchHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGWINCH, &sa, nullptr);
}

void Cli::Terminal::teardownSigwinch() {
    signal(SIGWINCH, SIG_DFL);
    winchPipeWriteFd.store(-1, std::memory_order_relaxed);
    if (sigPipe[0] != -1) close(sigPipe[0]);
    if (sigPipe[1] != -1) close(sigPipe[1]);
    sigPipe[0] = -1;
    sigPipe[1] = -1;
}

void Cli::Terminal::enableRawMode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &oldTermios);
    raw = oldTermios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |=  (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void Cli::Terminal::disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldTermios);
}

Cli::InputEvent Cli::Terminal::readEvent(int timeoutMs) const {
    while (true) {
        pollfd fds[2] = {
            {STDIN_FILENO, POLLIN, 0},
            {sigPipe[0], POLLIN, 0},
        };
        const nfds_t fdCount = sigPipe[0] == -1 ? 1 : 2;

        int result = poll(fds, fdCount, timeoutMs);
        if (result == 0) {
            return InputEvent::timeoutEvent();
        }

        if (result == -1) {
            if (errno == EINTR) continue;
            return InputEvent::keyEvent(Key::esc);
        }

        if (fdCount == 2 && (fds[1].revents & POLLIN)) {
            drainResizePipe();
            return InputEvent::resizeEvent();
        }

        if (fds[0].revents & POLLIN) {
            return InputEvent::keyEvent(readKeyFromStdin());
        }
    }
}


int Cli::Terminal::readKey() const {
    InputEvent event = readEvent();
    if (event.type == InputEventType::Resize) return Key::resize;
    return event.key;
}

int Cli::Terminal::readKeyFromStdin() const {
    char c;
    if (!readStdinByte(c, -1)) return Key::esc;

    if (c != '\033') return static_cast<int>(c);

    // Escape-последовательность: читаем остаток
    char seq[3] = {};
    if (!readStdinByte(seq[0], 25)) return Key::esc;
    if (!readStdinByte(seq[1], 25)) return Key::esc;

    if (seq[0] == '[') {
        // Стрелки: \033[A \033[B \033[C \033[D
        switch (seq[1]) {
            case 'A': return Key::arrowUp;
            case 'B': return Key::arrowDown;
            case 'C': return Key::arrowRight;
            case 'D': return Key::arrowLeft;
            case 'H': return Key::home;
            case 'F': return Key::end;
        }
        // Page Up/Down: \033[5~ \033[6~
        if (seq[1] == '5' || seq[1] == '6') {
            char tilde;
            if (!readStdinByte(tilde, 25)) return Key::esc;
            if (tilde == '~')
                return seq[1] == '5' ? Key::pageUp : Key::pageDown;
        }
    }

    return Key::esc;
}

bool Cli::Terminal::readStdinByte(char& c, int timeoutMs) const {
    while (true) {
        pollfd fd = {STDIN_FILENO, POLLIN, 0};
        int result = poll(&fd, 1, timeoutMs);

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

void Cli::Terminal::drainResizePipe() const {
    if (sigPipe[0] == -1) return;

    char buf[64];
    while (read(sigPipe[0], buf, sizeof(buf)) > 0) {}
}

void Cli::Terminal::refreshSize() {
    size = queryTermSize();
    centerRow = (size.rows / 2) + 1;
    centerCol = (size.cols / 2) + 1;
}

Cli::TermSize Cli::Terminal::queryTermSize() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return { w.ws_col, w.ws_row, w.ws_ypixel, w.ws_xpixel };
}
