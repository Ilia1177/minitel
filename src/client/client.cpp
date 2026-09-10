#include "Client.hpp"

Client::Client(): minitel(nullptr) {}

Client::Client(const char* device) 
{
	if (!this->serial.openPort(device))
		throw std::runtime_error(strerror(errno));
	this->minitel = new Minitel(this->serial);
}

std::string Client::current_page_str()
{
	switch(currentPage) {
		case MAIN_PAGE:
			return "main page";
		case RISO:
			return "game one";
		default:
			return "no know state";
	}
}

Client::Client(HardwareSerial& s): serial(s) 
{
	this->minitel = new Minitel(this->serial);
}

void Client::init(HardwareSerial& s) {
	this->minitel = new Minitel(s);
	serial = s;
}

// bool Client::displayPng(const std::string& path, int threshold) {
// 	if (!minitel) return false;
// 	return minitel->displayPng(path, threshold);
// }
//
// bool Client::displayPng(const std::string& path, int maxCellsW, int maxCellsH, int threshold) {
// 	if (!minitel) return false;
// 	return minitel->displayPng(path, maxCellsW, maxCellsH, threshold);
// }

// bool Client::displayPngCentered(const std::string& path, int threshold) {
// 	if (!minitel) return false;
// 	return minitel->displayPng(path, threshold);
// }
#include "Pty.hpp"
#include "TermScreen.hpp"
#include <termios.h>
#include <unistd.h>
void Client::killShell(bool fast)
{
    // If we were in Teleinfo, restore Videotex before closing
    if (pty && currentPage == SYSTEM) {
        if (minitel) {
            // Restore serial 7E1 first, then switch standard
            int fd = serial.getFd();
            termios tty{};
            if (fd >=0 && tcgetattr(fd, &tty)==0){
                cfmakeraw(&tty);
                // keep current baud, just fix parity/bits
                speed_t curIn = cfgetispeed(&tty);
                speed_t curOut = cfgetospeed(&tty);
                tty.c_cflag &= ~(CSIZE | CSTOPB | PARENB);
                tty.c_cflag |= CS7 | PARENB | CLOCAL | CREAD;
                tty.c_cc[VMIN]=0; tty.c_cc[VTIME]=0;
                cfsetispeed(&tty, curIn);
                cfsetospeed(&tty, curOut);
                tcsetattr(fd, TCSANOW, &tty);
                tcflush(fd, TCIOFLUSH);
            }
            if (fast) {
                // Fast path on signal: send switch bytes without waiting for ack
                // to avoid 2s timeouts in working*()
                extern volatile sig_atomic_t g_signal;
                // temporarily clear flag to allow raw write, then restore?
                // Directly write PRO sequence without ack wait via low-level
                // Fallback: try standardTeletel() which will abort quickly due to g_signal check
                // so just attempt it; it will return quickly.
                minitel->standardTeletel();
                minitel->pageMode();
            } else {
                usleep(100000);
                minitel->standardTeletel();
                usleep(200000);
                // Back to page mode for menus (undo scrollMode)
                minitel->pageMode();
                usleep(100000);
            }
        }
    }
    if (pty)  { pty->close_pty(); delete pty;  pty  = nullptr; }
    if (term) { delete term; term = nullptr; }
}

Client::~Client() {
	extern volatile sig_atomic_t g_signal;
	bool fast = (g_signal != 0);
	killShell(fast);
	if (!fast) {
		// minitel->echo(true);
		minitel->newScreen(); 
		minitel->println("Deconnexion.");
	} else {
		// Fast deconnexion on signal: try best effort without ack waits
		if (minitel) {
			// Don't wait for echo ack if signal
			// Send raw sequence without blocking
			// minitel->echo(true);
			minitel->newScreen();
			minitel->println("Deconnexion.");
		}
	}
	if (minitel)
		delete minitel;
	if (serial.getFd() > 0) {
		if (!fast) tcdrain(serial.getFd());
		::close(serial.getFd());
	}
}
