#include "Minitel.hpp"
#include <poll.h>
#include <vector>
#include <fcntl.h>
#include <unistd.h>

void Minitel::exec_choice(const std::string& rawCommand) {
	std::string cmd = rawCommand;
	// trim(cmd);<LeftMouse>
	// display_menu();
	std::cout << "Execute: " << cmd << "\n";
	if (cmd == "1" || cmd == "hazardous") {
		state_ = State::HAZARDOUS;
		hazardous_collective("");
	} else if (cmd == "2" || cmd == "story") {
		state_ = State::STORY;
		cadavre_exquis("");
	} else if (cmd == "3" || cmd == "42") {
		state_ = State::FORTY2;
		forty_two("");
	} else {
		send("invalid choice !! Try again: ");
	}
}

void Minitel::handle_input() {
	size_t pos;
	if ((pos = buffer_.find("\r")) != std::string::npos) {
		// clear_line();
		std::string input = buffer_.substr(0, pos);
		std::cout << "Handle input = buffer -> : " << buffer_  << " input: " << input << "\n";
		buffer_.erase(0, pos + 1);
		switch(state_) {
			case State::MENU: exec_choice(input); break;
			case State::HAZARDOUS: hazardous_collective(input); break;
			case State::STORY: 	cadavre_exquis(input); break;
			case State::FORTY2: forty_two(input); break;
			default: return;
		}
		if (state_ == State::MENU) {
			display_menu();
		}
	} else if ((pos = buffer_.find("\x13\x45")) != std::string::npos) {
		std::cout << "ANNULATION\n";
		clear_screen();
		buffer_.clear();
		g_interrupt = true;
	}
}

void Minitel::display_menu()
{
	std::cout << "Display menu\n";
	clear_screen();
	png_to_mosaique("ascii/img.png");
	std::vector<std::string> menu = {
			"       1. Hazardous Collective         ", 
			"       2. Tell me a story              ", 
			"       3. about 42                     " };

	for (size_t i = 0; i < menu.size(); i++) {
		send(BLACK_CHAR);
		send(WHITE_BCKG);
		send(menu[i]);
		send("\r\n");
	}
	send(WHITE_CHAR);
	send(BLACK_BCKG);
	blink_on();
	send("\r\nMake your choice: ");
	blink_off();

}

void Minitel::listen()
{
    struct pollfd pfd;
    char buf[32];
	int time = 0;

    pfd.fd = serial_port_;
    pfd.events = POLLIN;

	display_menu();
    while (!g_interrupt) {
		// flush();
        int ret = poll(&pfd, 1, 1000);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            std::cerr << "poll() error: " << strerror(errno) << "\n";
            break;
        } else if (ret == 0) {
			time = (time + 1) % 4;
			eraseLines(2);
			if (debugMode_) {
				std::cout << "DEBUG MODE: ";
				std::cout << "STATUS: " << get_state() + " " ;
			}
			std::cout << "Listening" << std::string(time, '.') << '\n';
			continue;
		}
        if (pfd.revents & POLLIN) {
            int n = ::read(serial_port_, buf, sizeof(buf) - 1);
			if (n == 0) {
				std::cerr << "Disconnection\n";
				break;
			} else if (n < 0) {
				std::cerr << "reception failed\n";
				break;
			} else if (n > 0) {
				std::vector<char> data(n);
				for (int i = 0; i < n; i++) {
					unsigned char byte = buf[i];
					buf[i] = byte & 0x7F; // Strip parity bit (bit 7)
					if (debugMode_) {
						if (::isprint(buf[i]))
							std::cout << "receive char: " << buf[i] << std::endl;
						else
							std::cout << "receive hex : " << std::hex << (int)buf[i] << std::endl;
					}
				}
				buffer_.append(buf, n);
				handle_input();
			} 

        } else if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
            std::cerr << "Poll error on serial port\n";
            break;
        }
    }
	std::cout << "Stop listening\n";
	g_interrupt = false;
}
