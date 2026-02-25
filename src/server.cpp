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

void Minitel::exec_choice(const std::string& input)
{
	if (input == "lovelace" || input == "ada" || input == "adalovelace") {
        _state = State::HAZARDOUS;
        hazardous_collective();
    	_buffer.clear();
		return;
	}
    int cmd = 0;
    auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), cmd);
    if (ec == std::errc::invalid_argument) {
        std::cerr << "Invalid command\n";
        _buffer.clear();
        return;
    } else if (ec == std::errc::result_out_of_range) {
        std::cerr << "Invalid command\n";
        _buffer.clear();
        return;
    }
    std::cout << "Execute: " << get_state() << "\n";
    switch (cmd) {
    case 1:
        _state = State::HAZARDOUS;
        hazardous_collective();
        break;
    case 2:
        _state = State::STORY;
        cadavre_exquis();
        break;
    case 3:
        _state = State::FORTY2;
        forty_two();
        break;
    default:
        send("Invalid choice ! You can try again...");
        break;
    }
    _buffer.clear();
}

bool Minitel::edition(unsigned char ch) {
	switch (ch) {
				case '\0': 
					return false;
				case 'E':
					std::cout << "input: ANNULATION\n";
					display_menu();
					_state = State::MENU;
					_buffer.clear();
					_rindex = 0;
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

void Minitel::update_cursor(int x, int y) {
	_cursorX = x % COLS_VIDEOTEX;
	_cursorY = std::clamp(y, 1, ROWS_VIDEOTEX);
}

bool Minitel::arrows(unsigned char ch) {
	size_t width = COLS_VIDEOTEX - (_margin * 2);
	switch(ch) {
		  case '\0': return false;
		  case 'A':
			std::cout << "input: ARROW up\n";
			if (_rindex >= width) {
				update_cursor(_cursorX, _cursorY - 1);
				send(CUR_UP);
				_cursorX--;
				_rindex -= width;
				std::cout << "index is at: " << _rindex << "\n";
			}
			return true;
		  case 'B':
			std::cout << "input: ARROW down\n";
			if (_rindex + width < _buffer.size()) {
				update_cursor(_cursorX, _cursorY - 1);
				send(CUR_DOWN);
				_rindex += width;
				std::cout << "index is at: " << _rindex << "\n";
			}
			return true;
		  case 'C':
			std::cout << "input: ARROW right\n";
			if (_rindex + 1 <= _buffer.size()) {
				update_cursor(_cursorX + 1, _cursorY);
				send(CUR_RIGHT);
				_rindex++;
				std::cout << "index is at: " << _rindex << "\n";
			}
			return true;
		  case 'D':
			std::cout << "input: ARROW <-\n";
			if (_rindex >= 1) {
				update_cursor(_cursorX - 1, _cursorY);
				send(CUR_LEFT);
				_rindex--;
				std::cout << "index is at: " << _rindex << "\n";
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
    std::cout << "-> INPUT: ()" << _rbuff.size() << "\n";
    static bool ctrl_seq = false;
    while (_rbuff.size() > 0) {
        // if we found ctrl_seq
        std::cout << "Process size: " << _rbuff.size() << "\n";
        // Check for Enter
        if (_rbuff[0] == '\r') {
            std::cout << "INPUT: '\\r', main buffer: '" << _buffer << "'\n";
            switch (_state) {
            case State::MENU:
                exec_choice(_buffer);
                break;
            case State::HAZARDOUS:
                hazardous_collective("exit");
                break;
            case State::STORY:
                cadavre_exquis(_buffer);
                break;
            case State::FORTY2:
                forty_two("exit");
                break;
            // case State::EMAIL: get_contact("exit"); break;
            default:
                break;
            }
            display_menu();
            _buffer.clear();              // ✓ Clear ONLY after Enter
            _rbuff.erase(_rbuff.begin()); // Remove the '\r'
            continue;
        } else if (::isprint(_rbuff[0]) && !ctrl_seq) {
            char c = _rbuff[0];
            std::cout << "input: add '" << c << "' to buffer\n";
            _buffer += c;
            write_text(std::string(1, c), _margin);      // Echo input to minitel
            _rbuff.erase(_rbuff.begin()); // Remove processed byte
			_rindex++;
			std::cout << "rindex at: " << std::dec << _rindex << "\n";
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
		_margin = margin;
	return (_paper + _ink);
}

std::string Minitel::get_typo() {
	return (_paper + _ink);
}


void Minitel::display_dialbox() {
	cursor_to(1, ROWS_VIDEOTEX);
    send(set_typo(BLACK_CHAR, MAGENTA_BCKG));
	send(" ->                                     ");
	cursor_to(1, 0);
	cursor_to(4, ROWS_VIDEOTEX);
    send(set_typo(BLACK_CHAR, MAGENTA_BCKG));
	std::cout << "Cursor moved at x: " << std::dec << _cursorX << " y: " << _cursorY <<"\n";
}

void Minitel::display_menu()
{
    if (_state != State::MENU) {
        return;
    }

    std::cout << "Display menu\n";
    send(CLEAR);
    png_to_mosaique("ascii/img.png");

    std::vector<std::string> menu = 
	{	"       1. Ada Lovelace.....             ",
        "       2. Write & code the future !     ",
        "       3. 424242424242.....             "
	};

	set_typo(WHITE_BCKG, BLACK_CHAR);
    for (size_t i = 0; i < menu.size(); i++) {
        send(get_typo());
        send(menu[i]);
		_cursorY++;
    }
	send(set_typo(WHITE_CHAR, BLUE_BCKG));
	send_file("ascii/welcome.txt");
	display_dialbox();
}

void Minitel::write_text(const std::string& text, int margin) {
	send(get_typo()); 
	for (size_t i = 0; i < text.size(); i++) {
		std::cout << std::dec << "cursorX: " << _cursorX << " cursorY: " << _cursorY << "\n";
		if (_cursorX == 1) {
			send(get_typo()); 
			while (_cursorX <= margin) {
				writeByte(' '); 
				_cursorX++; 
			}
		} else if (_cursorX == COLS_VIDEOTEX - (margin - 1)) {
			while(_cursorX <= COLS_VIDEOTEX) {
				writeByte(' ');
				_cursorX++;
			}
			_cursorX = 1;
			_cursorY++;
			i--;
			continue;
		}

		if (text[i] == '\r') {
			send(get_typo()); 
			_cursorX = 1; 
		} else if (text[i] == '\n') { 
			send(get_typo()); 
			_cursorY++;
		} else {
			_cursorX++;
		}
		writeByte(text[i]);
	}
	std::cout << std::dec << "RESULT: cursorX: " << _cursorX << " cursorY: " << _cursorY << "\n";
}

void Minitel::start()
{
    struct pollfd pfd;
    char          buf[32];

	_rindex = 0;
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
                std::cout << "-> READ: " << n << " byte(s)\n";
                for (int i = 0; i < n; i++) {
                    unsigned char byte = buf[i] & 0x7F;
                    if (::isprint(byte)) {
                        std::cout << "read: char'" << byte << "'\n";
                    } else {
                        std::cout << "read: hex '" << std::hex << (int)byte << "'\n";
                    }
                    _rbuff += byte;
                }
                handle_input();
            }

        } else if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
            std::cerr << "Poll error on serial port\n";
            break;
        }
        std::cout << "listening: STATUS: " << get_state() + "\n";
    }
    std::cout << "SERVER STOP\n";
    g_interrupt = false;
}
