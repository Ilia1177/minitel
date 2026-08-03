#ifndef HARDWARESIM_H
#define HARDWARESIM_H

#include <cstdint>
#include <string>
#include <chrono>
#include <thread>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

typedef uint8_t byte;
typedef uint16_t word;
typedef bool boolean;

#define SERIAL_7E1 0

#define highByte(w) ((byte)((w) >> 8))
#define lowByte(w)  ((byte)((w) & 0xFF))
#define bitRead(v,b) (((v) >> (b)) & 0x01)
#define bitWrite(value, bit, bitvalue) \
    ((bitvalue) ? ((value) |= (1UL << (bit))) : ((value) &= ~(1UL << (bit))))

inline unsigned long millis() {
    using namespace std::chrono;
    static auto t0 = steady_clock::now();
    return duration_cast<milliseconds>(steady_clock::now() - t0).count();
}

inline void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

class String : public std::string {
public:
    using std::string::string;
    String(const std::string& s) : std::string(s) {}
    char charAt(size_t i) const { return (*this)[i]; }
    int lastIndexOf(char c) const {
        auto pos = find_last_of(c);
        return pos == npos ? -1 : (int)pos;
    }
};
#include <cstring>   // strerror
#include <cerrno>
#include <iostream>

class HardwareSerial {
public:
    bool openPort(const char* device) {
        _fd = ::open(device, O_RDWR | O_NOCTTY);
        return _fd >= 0;
    }

    void begin(unsigned long baud, int /*config*/ = 0) {
        termios tty{};
        tcgetattr(_fd, &tty);
        cfmakeraw(&tty);
        speed_t s = baudToSpeed(baud);
        cfsetispeed(&tty, s);
        cfsetospeed(&tty, s);
        tty.c_cflag |= (CLOCAL | CREAD);
        tty.c_cflag |= PARENB;
        tty.c_cflag &= ~PARODD;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS7;
        tty.c_cc[VMIN]  = 0;
        tty.c_cc[VTIME] = 5;
        tcsetattr(_fd, TCSANOW, &tty);
    }

    void end() {}

    size_t write(uint8_t b) { return ::write(_fd, &b, 1); }

    int available() {
        int n = 0;
		if (ioctl(_fd, FIONREAD, &n) < 0) {
			return 0;
		}
        return n;
    }

    int read() {
        uint8_t b;
        return (::read(_fd, &b, 1) == 1) ? b : -1;
    }

    explicit operator bool() const { return _fd >= 0; }

	int getFileDescriptor() {return _fd;}

private:
    int _fd = -1;
    speed_t baudToSpeed(unsigned long baud) {
        switch (baud) {
            case 300:  return B300;
            case 1200: return B1200;
            case 4800: return B4800;
            case 9600: return B9600;
            default:   return B1200;
        }
    }
};

#endif
