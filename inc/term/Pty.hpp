// Pty.hpp
#pragma once

#ifdef __APPLE__
#include <util.h>
#else
#include <pty.h>
#endif

#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <string>
#include <cstring>
#include <signal.h>
#include <sys/wait.h>

class Pty {
public:
    Pty() = default;
    ~Pty() { close_pty(); }

    bool spawn(int cols, int rows, const char* shellPath = "/bin/bash")
    {
        struct winsize ws{};
        ws.ws_col = cols;
        ws.ws_row = rows;

        pid = forkpty(&masterFd, nullptr, nullptr, &ws);
        if (pid < 0) return false;

        if (pid == 0) {
            // child
            setenv("TERM", "vt100", 1);
            setenv("PS1", "\\W $ ", 1); // keep prompt short for 40 cols
            struct termios tio{};
            tcgetattr(STDIN_FILENO, &tio);
            tio.c_lflag &= ~ECHO;      // we do echo ourselves via Minitel emulation
            tcsetattr(STDIN_FILENO, TCSANOW, &tio);
            execl(shellPath, shellPath, "--login", nullptr);
            _exit(127);
        }
        return true;
    }

    void resize(int cols, int rows)
    {
        if (masterFd < 0) return;
        struct winsize ws{};
        ws.ws_col = cols;
        ws.ws_row = rows;
        ioctl(masterFd, TIOCSWINSZ, &ws);
    }

    int  getFd() const { return masterFd; }
    pid_t getPid() const { return pid; }

    ssize_t write(const char* buf, size_t len)
    {
        return masterFd >= 0 ? ::write(masterFd, buf, len) : -1;
    }

	void close_pty()
	{
		if (masterFd >= 0) { ::close(masterFd); masterFd = -1; }
		if (pid > 0) {
			kill(pid, SIGHUP);
			int status;
			// give it a moment, then reap; don't block forever if it's already gone
			for (int tries = 0; tries < 50; tries++) {
				pid_t r = waitpid(pid, &status, WNOHANG);
				if (r == pid || r < 0) break;
				usleep(10000);
			}
			pid = -1;
		}
	}

    bool active() const { return masterFd >= 0; }

private:
    int   masterFd = -1;
    pid_t pid = -1;
};
