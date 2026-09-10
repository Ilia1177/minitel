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
            // child — Teleinfo is transparent vt100, keep 7-bit clean
            setenv("TERM", "vt100", 1);
            setenv("LANG", "C", 1);
            setenv("LC_ALL", "C", 1);
            setenv("LC_CTYPE", "C", 1);
            // Avoid utf8 prompts that become white blocks on 7E1 Minitel
            if (cols >= 80) setenv("PS1", "\\u@\\h:\\w $ ", 1);
            else setenv("PS1", "\\W $ ", 1);
            // Leave termios to default (ECHO on) for Teleinfo passthrough
            execl(shellPath, shellPath, nullptr);
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
