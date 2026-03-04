#include "Minitel.hpp"
#include <charconv>
#include <fcntl.h>
#include <poll.h>
#include <string>
#include <string_view>
#include <system_error>
#include <termios.h>
#include <unistd.h>
#include <vector>

#include "IndexPage.hpp"
#include "MazePage.hpp"
#include "CadavrePage.hpp"
#include "ContactPage.hpp"
#include "Forty2Page.hpp"

bool Minitel::edition(unsigned char ch) {
	switch (ch) {
				case '\0': 
					return false;
				case 'E':
					std::cout << "input: ANNULATION\n";
					index->display();
					_state = State::MENU;
					_buffer.clear();
					_bufind = 0;
					return true;
				case 'G':
					std::cout << "input: CORRECTION\n";
					send(get_typo());
					if (_buffer.size() > 0) {
						_buffer.erase(_buffer.end() - 1);
						if (cursorX == 2) {
							while(cursorX >= 0) {
								send(BS);
								writeByte(' ');
								send(BS);
								cursorX--;
							}
							cursorY--; 
							cursorX = 39;
						} else {
							cursorX--;
							send(BS);
							send(" ");
							send(BS);
						}
					}
					return true;
				default:
					std::cerr << "Comand not handle\n";
					return true;
			}
	return true;
}

void::Minitel::scrolldown()
{
	bool nextLine = cursorY == 25 ? true : false;

	if (!nextLine)
		cursor_to(cursorX, 25);
	send(CUR_UP);
	if (!nextLine)
		cursor_to(cursorX, cursorY);
}

void::Minitel::scrollup() {
	bool nextLine = cursorY == 1 ? true : false;

	if (!nextLine) 
		cursor_to(cursorX, 1);
	send(CUR_UP);
	if (!nextLine)
		cursor_to(cursorX, cursorY);
}

// x = _cusorX - 5; y = cursorY + 6;
void Minitel::update_cursor(int destX, int destY, int margin)
{
	bool needMoveX = false;
	bool needMoveY = false;
	
	// dest is < margin
	if (destX < margin + 1) {
		for (int i = 0; i < margin * 2; i++) {
			if (i == margin / 2) send(get_typo());
			send(CUR_LEFT);
		}
		send(CUR_UP);
		if (cursorY > 1) cursorY--; destY--;
		cursorX = COLS_VIDEOTEX - margin;
	} else if (destX > COLS_VIDEOTEX - margin) {
		for (int i = 0; i < margin * 2; i++) {
			if (i == margin / 2) send(get_typo());
			send(CUR_RIGHT);
		}
		if (cursorY < ROWS_VIDEOTEX) cursorY++; destY++;
		cursorX = margin + 1;
	} else if (cursorX != destX) {
		needMoveX = true;
	}

	if (destY <= 0) {
		send(CUR_UP);
		if (cursorY > 1) cursorY--;
	} else if (destY > ROWS_VIDEOTEX) {
		send(CUR_DOWN);
		std::cout << "Send cursor down\n";
		if (cursorY < ROWS_VIDEOTEX) cursorY++;
	} else if (cursorY != destY) {
		needMoveY = true;
	}

	if (needMoveX && needMoveY) {
		cursor_to(destX, destY);
	} else if (needMoveX) {
		cursor_to(destX, cursorY);
	} else if (needMoveY) {
		cursor_to(cursorX, destY);
	}
	// std::cout << "update cursor to x: " << std::dec << cursorX << " y: " << cursorY << "\n";
}

// void Minitel::update_cursor(int x, int y) {
// 	cursorX = x % COLS_VIDEOTEX ;
// 	cursorY = std::clamp(y, 1, ROWS_VIDEOTEX);
// 	std::cout << "update cursor to x: " << std::dec << cursorX << " y: " << cursorY << "\n";
// }
//
void Minitel::rules() {
	switch(_state) {
		case State::STORY :
		case State::MENU :
			if (cursorX == _marginX) {
				for (int i = 0; i < _marginX; i++) {
					send(CUR_RIGHT);
				}
				// update_cursor(40, cursorY - 1);
			}
			if (cursorX >= COLS_VIDEOTEX - _marginX) {
				for (int i = 0; i < _marginX; i++) {
					send(CUR_RIGHT);
				}
				// update_cursor(40, cursorY - 1);
			}
		case State::HAZARDOUS:
		default: return;
	}
}

bool Minitel::arrows(unsigned char ch) {
	size_t width = COLS_VIDEOTEX;
	switch(ch) {
		  case '\0': return false;
		  case 'A':
			std::cout << "input: ARROW up\n";
			if (_bufind >= width) {
				update_cursor(cursorX, cursorY - 1, _marginX);
				_bufind -= width;
				std::cout << "update buffer index: " << std::dec << _bufind << "\n";
			}
			return true;
		  case 'B':
			std::cout << "input: ARROW down\n";
			if (_bufind + width <= _buffer.size()) {
				update_cursor(cursorX, cursorY + 1, _marginX);
				_bufind += width;
				std::cout << "update buffer index: " << std::dec << _bufind << "\n";
			} return true;
		  case 'C':
			std::cout << "input: ARROW ->\n";
			if (!_buffer.empty() && _bufind < _buffer.size()) {
					update_cursor(cursorX + 1, cursorY, _marginX);
					_bufind++;
					std::cout << "update buffer index: " << std::dec << _bufind << "\n";
			}; return true;
		  case 'D':
			std::cout << "input: ARROW <-\n";
			if (_bufind >= 1 && _buffer.size() >= 1) {
				update_cursor(cursorX - 1, cursorY, _marginX);
				_bufind--;
			}
			return true;
		  default:
			std::cerr << "Comand not handle\n";
			return true;
	  };
	return true;
}

bool Minitel::handle_controle_sequence()
{
    std::cout << "handle controle sequence\n";
    if (!::isprint(_rbuff[0])) { // handle all command char after '\r' has been executed
        switch (_rbuff[0]) {
        case MT_ESC:
			switch(_rbuff[1]) {
				case '\0': return false;
				case '[': 
					if (!arrows(_rbuff[2])) 
						return false;
					_rbuff.erase(0, 3);
				default : _rbuff.clear(); return true;
			} 
        case MT_DC3:
            if (!edition(_rbuff[1]))
				return false;
			_rbuff.erase(0, 2);
			return true;
        default: 
			std::cerr << "Comand not handle\n";
            _rbuff.clear();
			return true;;
        }
    }
    return false;
}

void Minitel::handle_input()
{
	static State current = _state;
    // std::cout << "-> INPUT: ()" << _rbuff.size() << "\n";
    while (_rbuff.size() > 0) {
        // std::cout << "Process size: " << _rbuff.size() << "\n";
        if (_rbuff[0] == '\r') {
            // std::cout << "INPUT: '\\r', main buffer: '" << _buffer << "'\n";
            switch (current) {
				case State::MENU:
					current = index->handle_input(_buffer); break;
				case State::HAZARDOUS:
					current = maze->handle_input(_buffer); break;
				case State::STORY:
					current = cadavre->handle_input(_buffer); break;
				case State::FORTY2:
					current = forty2->handle_input(_buffer); break;
				case State::EMAIL: 
					current = contact->handle_input(_buffer); break;
				default: break;
            }
            _buffer.clear();              // Clear ONLY after Enter
			_bufind = 0;
            _rbuff.erase(_rbuff.begin()); // Remove the '\r'
            continue;
        } else if (::isprint(_rbuff[0])) {
            char c = _rbuff[0];
            // std::cout << "input: add '" << c << "' to buffer\n";
			if (_bufind >= _buffer.size()) {
            	_buffer += c;
			} else if (_bufind < _buffer.size()) {
				_buffer[_bufind] = c;
			}
            write_text(std::string(1, c), _marginX);      // Echo input to minitel
            _rbuff.erase(_rbuff.begin()); // Remove processed byte
			_bufind++;
			// std::cout << "rindex at: " << std::dec << _bufind << "\n";
			continue;
        }
		if (!handle_controle_sequence())
			break;
    }
}

std::string Minitel::set_typo(const std::string& bck, const std::string& ch, int margin) {
	if (!bck.empty())
		_paper = bck;
	if (!bck.empty())
		_ink = ch;
	if (margin >= 0 && margin < COLS_VIDEOTEX / 2)
		_marginX = margin;
	return (_paper + _ink);
}

std::string Minitel::get_typo() {
	return (_paper + _ink);
}


void Minitel::display_dialbox() {
	cursor_to(1, ROWS_VIDEOTEX);
    send(set_typo(WHITE_CHAR, BLACK_BCKG));
	write_text("->", 0, TRUNC);
}

void Minitel::display_menu()
{
    std::cout << "Display menu\n";
}

void Minitel::start()
{
    struct pollfd pfd;
    char          buf[32];

	_marginX = 1;
	_bufind = 0;
    pfd.fd = _serial_port;
    pfd.events = POLLIN;
    tcflush(_serial_port, TCIOFLUSH);
    // int time = 0;
	_rbuff.clear();
	index->display();
	_state = State::MENU;
    display_menu();
    // bool printable = true;
	std::cout << "start Listening LOOP\n";
    while (!g_interrupt) {

        int ret = poll(&pfd, 1, 1000);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            std::cerr << "poll() error: " << strerror(errno) << "\n";
            break;
        } else if (ret == 0) {
            continue;
        }

        if (pfd.revents & POLLIN) {
            int n = ::read(_serial_port, buf, sizeof(buf));
            if (n == 0) {
                std::cerr << "Disconnection\n";
                break;
            } else if (n < 0) {
                std::cerr << "reception failed\n";
                break;
            } else if (n > 0) {
                // std::cout << "-> READ: " << n << " byte(s)\n";
                for (int i = 0; i < n; i++) {
                    unsigned char byte = buf[i] & 0x7F;
                    if (::isprint(byte)) {
                        // std::cout << "read: char'" << byte << "'\n";
                    } else {
                        // std::cout << "read: hex '" << std::hex << (int)byte << "'\n";
                    }
                    _rbuff += byte;
                }
                handle_input();
            }

        } else if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
            std::cerr << "Poll error on serial port\n";
            break;
        }
        std::cout << "listening: STATUS: " << get_state(_state) + "\n";
    }
    std::cout << "SERVER STOP\n";
    g_interrupt = false;
}
