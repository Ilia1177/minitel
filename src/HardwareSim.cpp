#include "HardwareSim.h"

bool HardwareSerial::openPort(const char* device)
{
    _fd = ::open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (_fd < 0)
		std::cerr << "Fail opening port: " << strerror(errno) << "\n";
    return _fd >= 0;
}

// void HardwareSerial::begin(unsigned long baud, int config)
// {
//     (void)config;
//     termios tty{};
//     tcgetattr(_fd, &tty);
//     cfmakeraw(&tty);
//     speed_t s = baudToSpeed(baud);
//     cfsetispeed(&tty, s);
//     cfsetospeed(&tty, s);
//     tty.c_cflag |= (CLOCAL | CREAD);
//     tty.c_cflag |= PARENB;
//     tty.c_cflag &= ~PARODD;
//     tty.c_cflag &= ~CSTOPB;
//     tty.c_cflag &= ~CSIZE;
//     tty.c_cflag |= CS7;
//     tty.c_cc[VMIN] = 0;
//     tty.c_cc[VTIME] = 5;
//     tcsetattr(_fd, TCSANOW, &tty);
// }

void HardwareSerial::begin(unsigned long baud, int config)
{
    (void)config;

    termios tty{};
    tcgetattr(_fd, &tty);

    cfmakeraw(&tty);

    speed_t s = baudToSpeed(baud);
    cfsetispeed(&tty, s);
    cfsetospeed(&tty, s);

    tty.c_cflag |= (CLOCAL | CREAD);

    // Minitel: 7E1
    tty.c_cflag |= PARENB;
    tty.c_cflag &= ~PARODD;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS7;

    // Completely non-blocking reads
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 0;

    tcsetattr(_fd, TCSANOW, &tty);

    // Also make the file descriptor non-blocking
    int flags = fcntl(_fd, F_GETFL, 0);
    fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
}

void HardwareSerial::end() {}

size_t HardwareSerial::write(uint8_t b)
{
    return ::write(_fd, &b, 1);
}

int HardwareSerial::available()
{
    int n = 0;
    if (ioctl(_fd, FIONREAD, &n) < 0) {
        return 0;
    }
    return n;
}

int HardwareSerial::read()
{
    uint8_t b;
    return (::read(_fd, &b, 1) == 1) ? b : -1;
}

HardwareSerial::operator bool() const 
{ 
	return _fd >= 0; 
}

int HardwareSerial::getFileDescriptor() 
{
	return _fd; 
}

speed_t HardwareSerial::baudToSpeed(unsigned long baud)
{
    switch (baud) {
    case 300:
        return B300;
    case 1200:
        return B1200;
    case 4800:
        return B4800;
    case 9600:
        return B9600;
    default:
        return B1200;
    }
}
