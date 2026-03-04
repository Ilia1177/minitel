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

bool Minitel::edition(unsigned char ch) {
	switch (ch) {
				case '\0': 
					return false;
				case 'E':
					std::cout << "input: ANNULATION\n";
					display_menu();
					_state = State::MENU;
					_buffer.clear();
					_bufind = 0;
					return true;
				case 'G':
					std::cout << "input: CORRECTION\n";
					send(get_typo());
					if (_buffer.size() > 0) {
						_buffer.erase(_buffer.end() - 1);
						if (_cursorX == 2) {
							while(_cursorX >= 0) {
								send(BS);
								writeByte(' ');
								send(BS);
								_cursorX--;
							}
							_cursorY--; 
							_cursorX = 39;
						} else {
							_cursorX--;
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
	bool nextLine = _cursorY == 25 ? true : false;

	if (!nextLine)
		cursor_to(_cursorX, 25);
	send(CUR_UP);
	if (!nextLine)
		cursor_to(_cursorX, _cursorY);
}

void::Minitel::scrollup() {
	bool nextLine = _cursorY == 1 ? true : false;

	if (!nextLine) 
		cursor_to(_cursorX, 1);
	send(CUR_UP);
	if (!nextLine)
		cursor_to(_cursorX, _cursorY);
}

// x = _cusorX - 5; y = _cursorY + 6;
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
		if (_cursorY > 1) _cursorY--; destY--;
		_cursorX = COLS_VIDEOTEX - margin;
	} else if (destX > COLS_VIDEOTEX - margin) {
		for (int i = 0; i < margin * 2; i++) {
			if (i == margin / 2) send(get_typo());
			send(CUR_RIGHT);
		}
		if (_cursorY < ROWS_VIDEOTEX) _cursorY++; destY++;
		_cursorX = margin + 1;
	} else if (_cursorX != destX) {
		needMoveX = true;
	}

	if (destY <= 0) {
		send(CUR_UP);
		if (_cursorY > 1) _cursorY--;
	} else if (destY > ROWS_VIDEOTEX) {
		send(CUR_DOWN);
		std::cout << "Send cursor down\n";
		if (_cursorY < ROWS_VIDEOTEX) _cursorY++;
	} else if (_cursorY != destY) {
		needMoveY = true;
	}

	if (needMoveX && needMoveY) {
		cursor_to(destX, destY);
	} else if (needMoveX) {
		cursor_to(destX, _cursorY);
	} else if (needMoveY) {
		cursor_to(_cursorX, destY);
	}
	// std::cout << "update cursor to x: " << std::dec << _cursorX << " y: " << _cursorY << "\n";
}

// void Minitel::update_cursor(int x, int y) {
// 	_cursorX = x % COLS_VIDEOTEX ;
// 	_cursorY = std::clamp(y, 1, ROWS_VIDEOTEX);
// 	std::cout << "update cursor to x: " << std::dec << _cursorX << " y: " << _cursorY << "\n";
// }
//
void Minitel::rules() {
	switch(_state) {
		case State::STORY :
		case State::MENU :
			if (_cursorX == _marginX) {
				for (int i = 0; i < _marginX; i++) {
					send(CUR_RIGHT);
				}
				// update_cursor(40, _cursorY - 1);
			}
			if (_cursorX >= COLS_VIDEOTEX - _marginX) {
				for (int i = 0; i < _marginX; i++) {
					send(CUR_RIGHT);
				}
				// update_cursor(40, _cursorY - 1);
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
				update_cursor(_cursorX, _cursorY - 1, _marginX);
				_bufind -= width;
				std::cout << "update buffer index: " << std::dec << _bufind << "\n";
			}
			return true;
		  case 'B':
			std::cout << "input: ARROW down\n";
			if (_bufind + width <= _buffer.size()) {
				update_cursor(_cursorX, _cursorY + 1, _marginX);
				_bufind += width;
				std::cout << "update buffer index: " << std::dec << _bufind << "\n";
			} return true;
		  case 'C':
			std::cout << "input: ARROW ->\n";
			if (!_buffer.empty() && _bufind < _buffer.size()) {
					update_cursor(_cursorX + 1, _cursorY, _marginX);
					_bufind++;
					std::cout << "update buffer index: " << std::dec << _bufind << "\n";
			}; return true;
		  case 'D':
			std::cout << "input: ARROW <-\n";
			if (_bufind >= 1 && _buffer.size() >= 1) {
				update_cursor(_cursorX - 1, _cursorY, _marginX);
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
	static State current;
    // std::cout << "-> INPUT: ()" << _rbuff.size() << "\n";
    while (_rbuff.size() > 0) {
        // std::cout << "Process size: " << _rbuff.size() << "\n";
        if (_rbuff[0] == '\r') {
            // std::cout << "INPUT: '\\r', main buffer: '" << _buffer << "'\n";
            switch (current) {
				case State::MENU:
					current = index_page(_state, _buffer); break;
				case State::HAZARDOUS:
					current = hazardous_collective(_state, _buffer); break;
				case State::STORY:
					current = cadavre_exquis(_state, _buffer); break;
				case State::FORTY2:
					current = forty_two(_state, _buffer); break;
				case State::EMAIL: 
					current = add_contact(_state, _buffer); break;
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
    send(set_typo(BLACK_CHAR, MAGENTA_BCKG));
	send(" ->                                     ");
	scrollup();
	update_cursor(4, ROWS_VIDEOTEX, 0);
    send(set_typo(BLACK_CHAR, MAGENTA_BCKG));
}

void Minitel::display_menu()
{
    std::cout << "Display menu\n";
	_cursorX = 1;
	_cursorY = 1;

	set_typo(WHITE_BCKG, BLACK_CHAR);
    for (size_t i = 0; i < menu.size(); i++) {
        send(get_typo());
        send(menu[i]);
		if (_cursorY < ROWS_VIDEOTEX) _cursorY++;
    }
	send(set_typo(WHITE_CHAR, BLUE_BCKG));
	send_file("ascii/welcome.txt");
	display_dialbox();
}

size_t nextWordLength(const std::string& str, size_t pos = 0) {
    size_t start = str.find_first_not_of(" \r\n\t", pos);
    if (start == std::string::npos) return 0;
	size_t end = str.find_first_of(" \r\n\t", start);
    if (end == std::string::npos) end = str.size();
    return end - start;
}

void Minitel::write_text(const std::string& text, int margin, EditionMode align) {
	send(get_typo());

	const int lineWidth = COLS_VIDEOTEX - margin * 2;
	const int colStart = margin + 1;
	const int colEnd = COLS_VIDEOTEX - margin;

	int len = nextWordLength(text, 0);
	if (align == LEFT)
		std::cout << "cursor x: " << _cursorX << std::endl;
	for (size_t i = 0; i < text.size(); i++) {
		// handle right margin first -> finish on newline
		if (_cursorX > colEnd) {
			for (int i = 0; i < margin; i++) 
				writeByte(' ');
			if (_cursorY < ROWS_VIDEOTEX) 
				_cursorY++;
			_cursorX = 1;
		}

		// If the word is too long for the space left, write it on new line (if its longer than the width)
		if (align == LEFT && _cursorX + len > colEnd + 1 && len <= lineWidth) {
			for (int i = _cursorX; i <= COLS_VIDEOTEX; i++) writeByte(' ');
			if (_cursorY < ROWS_VIDEOTEX) 
				_cursorY++;
			_cursorX = 1;
		}

		// finally handle left margin
		if (_cursorX < colStart) {
			send(get_typo());
			for (int i = 0; i < margin; i++) writeByte(' ');
			_cursorX = colStart;
			// while (text[i] == ' ')
			// 	i++;
		}
		writeByte(text[i]);
		if (text[i] == '\r') {
			_cursorX = 1; 
		} else if (text[i] == '\n') { 
			send(get_typo()); 
			if (_cursorY < ROWS_VIDEOTEX) _cursorY++;
		} else {
			_cursorX++;
			if (text[i] == ' ')
        		len = nextWordLength(text, i + 1);
		}
	}
	// std::cout << std::dec << "write_text: cursorX: " << _cursorX << " cursorY: " << _cursorY << "\n";
}

// void Minitel::write_text(const std::string& text, int margin, EditionMode align) {
// 	send(get_typo());
//
	// int len = nextWordLength(text, 0);
// 	for (size_t i = 0; i < text.size(); i++) {
// 		// handle left margin -> write two spaces
// 		if (_cursorX == 1) {
// 			send(get_typo()); 
// 			while (_cursorX <= margin) {
// 				writeByte(' '); 
// 				_cursorX++; 
// 			}
// 		// handle right margin -> finish on newline
// 		} else if (_cursorX >= COLS_VIDEOTEX - (margin - 1)) {
// 			while(_cursorX <= COLS_VIDEOTEX) {
// 				writeByte(' ');
// 				_cursorX++;
// 			}
// 			_cursorX = 1;
// 			if (_cursorY < ROWS_VIDEOTEX) _cursorY++;
// 			i--; // because we continue, but we didnt write the current char at i
// 			continue; // Can with skip continue ? maybe
// 		} else if (align == LEFT && _cursorX + len > COLS_VIDEOTEX - margin && len < COLS_VIDEOTEX - margin * 2) {
// 			while (_cursorX <= COLS_VIDEOTEX) { 
// 				writeByte(' ');
// 				_cursorX++;
// 			}
// 			_cursorX = 1;
// 			if (_cursorY < ROWS_VIDEOTEX) _cursorY++;
// 			i--;
// 		}
//
// 		if (text[i] == '\r') {
// 			send(get_typo()); 
// 			_cursorX = 1; 
// 		} else if (text[i] == '\n') { 
// 			send(get_typo()); 
// 			if (_cursorY < ROWS_VIDEOTEX) _cursorY++;
// 		} else {
// 			_cursorX++;
// 		}
// 		writeByte(text[i]);
// 	}
// 	// std::cout << std::dec << "write_text: cursorX: " << _cursorX << " cursorY: " << _cursorY << "\n";
// }

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
