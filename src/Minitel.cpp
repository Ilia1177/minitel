#include "Minitel.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <png.h>
#include <fcntl.h>
#include <termios.h>
#include <cstdio>
#include <cstring>
#include <sys/ioctl.h>
// #include "videotex-cmd.hpp"

#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOCFPlugIn.h>

#include "IndexPage.hpp"    // ← add these
#include "MazePage.hpp"
#include "CadavrePage.hpp"
#include "ContactPage.hpp"
#include "Forty2Page.hpp"

Minitel::Minitel(void): 
	cursorX(1),
	cursorY(1),
	_paper(BLACK_BCKG),
	_ink(WHITE_CHAR),
	_marginX(1),
	_bufind(0),
	_serial_port(-1),
	_mode(VIDEOTEX),
	_state(State::MENU),
	_rbuff(""),
	_buffer(""),
	_story(""),
	_debugMode(false) {
		index = new IndexPage(this);
		maze = new MazePage(this);
		contact = new ContactPage(this);
		forty2 = new Forty2Page(this);
		cadavre = new CadavrePage(this);
	}

Minitel::~Minitel(void) {
	if (index)
		delete index;
	if (maze)
		delete maze;
	if (contact)
		delete contact;
	if (forty2)
		delete forty2;
	if (cadavre)
		delete cadavre;

	this->close();
	if (_storyBook.is_open()) {
		_storyBook.close();
		std::cout << "Close story book.\n";
	}
	if (_contacts.is_open()) {
		_storyBook.close();
		std::cout << "Close story book.\n";
	}
	if (_printer) {
		_printer->close_device();
		delete _printer;
	}
}

std::string Minitel::get_state(State state) {
	std::string str;
	if (state == State::MENU) {
		str = "MENU";
	} else if (state == State::HAZARDOUS) {
		str=  "HAZARDOUS";
	} else if (state == State::STORY) {
		str = "STORY";
	} else if (state == State::FORTY2) {
		str = "FORTY2";
	} else if (state == State::EMAIL) {
		str = "EMAIL";
	}
	return str;
}

int Minitel::init(int ac, char** av) {

	bool minitel = true;
	if (ac > 2 && std::string(av[2]) == "-d") {
		_debugMode = true;
	} else if (ac < 2 && std::string(av[2]) == "-d") {
		_debugMode = true;
		_serial_port = 0;
		minitel = false;
	} else if (ac < 2) {
		std::cerr << "Please provide a socket.\n";
		return 1;
	}

	if (minitel && configure_serial(av[1]) < 0) {
		return -1;
	}

	std::cout << "init Minitel with sequence: " << INIT << "\n";
	send(INIT);
	return 0;
}

int Minitel::changeSpeed(int bauds) {  // Voir p.141
  // Fonction modifiée par iodeo sur GitHub en octobre 2021
  // Format de la commande
  // send(PRO)
  send(PRO2);  // 0x1B 0x3A
  writeByte(0x6B);   // 0x6B
  switch (bauds) {
    case  300 : writeByte(0x52); break;  // 0x52
    // case  300 : writeByte(0b1010010); break;  // 0x52
    case 1200 : writeByte(0x64); break;  // 0x64
    // case 1200 : writeByte(0b1100100); break;  // 0x64
    case 4800 : writeByte(0x76); break;  // 0x76
    // case 4800 : writeByte(0b1110110); break;  // 0x76
    case 9600 : writeByte(0x7F); break;  // 0x7F (pour le Minitel 2 seulement)
    // case 9600 : writeByte(0b1111111); break;  // 0x7F (pour le Minitel 2 seulement)
  }
  return 1;
}

int Minitel::configure_serial(const char* port) {
	// NONBLOCK because of poll (taht allready block for incoming data) ???
	std::cout << "Configure SERIAL PORT\n";
    _serial_port = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (_serial_port == -1) {
        perror("Erreur ouverture port");
		return -1;
    }
	usleep(500000); // 500ms — actually let USB enumerate
	tcgetattr(_serial_port, &original_termios_);

    tcflush(_serial_port, TCIOFLUSH);  // Flush both input and output
    // Remove O_NONBLOCK after opening
    int flags = fcntl(_serial_port, F_GETFL, 0);
    fcntl(_serial_port, F_SETFL, flags & ~O_NONBLOCK);
    
    // Flush any existing data BEFORE configuring
    
    // Get current settings and clear them completely
    struct termios options;
	if (tcgetattr(_serial_port, &options) != 0) {
        perror("tcgetattr");
        return -1;
    }
    
	std::cout << "\tset IO speed\n";
    // Clear all flags to start fresh
    cfmakeraw(&options);  // Sets up raw mode cleanly
    cfsetospeed(&options, B1200);   // Output speed ← ADD THIS!
    // Set baud rate
    cfsetispeed(&options, B1200);
	//    // CS8, no parity - macOS compatible
	// options.c_cflag &= ~CSIZE;    // Clear size bits
	// options.c_cflag |= CS8;       // 8 bits (macOS compatible)
	// options.c_cflag &= ~PARENB;   // No hardware parity
	// options.c_cflag &= ~PARODD;   // (just in case)
	// options.c_cflag &= ~CSTOPB;   // 1 stop bit
	// options.c_cflag &= ~CRTSCTS;  // No flow control
	// options.c_cflag |= CREAD;     // Enable receiver
	// options.c_cflag |= CLOCAL;    // Ignore modem lines


	std::cout << "\tconfig 7E1...\n";
    // 7E1 configuration
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS7;      // 7 data bits
	options.c_cflag |= PARENB;   // Enable parity
	options.c_cflag &= ~PARODD;  // Even parity
	options.c_cflag |= CREAD;   
	options.c_cflag |= CLOCAL;  

	options.c_iflag  = 0;         // No input processing
	options.c_oflag  = 0;         // No output processing
	options.c_lflag  = 0;         // Raw mode

	options.c_cc[VMIN]  = 0;
	options.c_cc[VTIME] = 0;

	std::cout << "\tapply settings with TCSAFLUSH...\n";
    // Apply settings with TCSAFLUSH to discard data
    tcsetattr(_serial_port, TCSAFLUSH, &options);
    
	std::cout << "\tTCIOFLUSH...\n";
    // Flush again after configuration
    tcflush(_serial_port, TCIOFLUSH);
    
    // DTR/RTS control - toggle to reset Minitel state
	std::cout << "\tDTR/RTS control\n";
    int status;
    ioctl(_serial_port, TIOCMGET, &status);
    status |= TIOCM_DTR | TIOCM_RTS;
    ioctl(_serial_port, TIOCMSET, &status);
    
    // Wait for Minitel to be ready
    usleep(500000);  // 500ms

	// STEP 3: Read and discard anything that arrived
    // char discard_buf[256];
    // int discarded = 0;
    // int n;

    // std::cout << "Attempt to read on RX incoming garbage data\n";
    // while ((n = ::read(_serial_port, discard_buf, sizeof(discard_buf))) > 0) {
    //     discarded += n;
    //     std::cout << "Discarding " << n << " startup bytes: ";
    //     for (int i = 0; i < n; i++) {
    //         printf("%02X ", (unsigned char)discard_buf[i]);
    //     }
    //     std::cout << "\n";
    //     usleep(80000);  // Small delay between reads
    // }
    // std::cout << "Discarded " << discarded << " startup bytes total\n";
    
	struct termios current;
	tcgetattr(_serial_port, &current);
	tcflush(_serial_port, TCIOFLUSH);             // Belt-and-suspenders flush input
	std::cout << "c_cflag: 0x" << std::hex << current.c_cflag << "\n";
	std::cout << "CS7 set: " << ((current.c_cflag & CSIZE) == CS7) << "\n";
	std::cout << "PARENB set: " << ((current.c_cflag & PARENB) != 0) << "\n";
	std::cout << "PARODD set: " << ((current.c_cflag & PARODD) != 0) << "\n\n";
	std::cout << "CS8 set: " << ((current.c_cflag & CSIZE) == CS8) << "\n";
	std::cout << "PARENB: "  << ((current.c_cflag & PARENB) != 0) << "\n";
    return 1;
}

size_t Minitel::dial_menu(const std::vector<std::string>& menu)
{
	int margin = 4;
	if (g_interrupt)
		return 0;
	int choice = 0;
	for (size_t i = 0; i < menu.size(); i++) {
		std::cout << fit(" ", margin) << i + 1 << ". " << menu[i] << "\n";
	}
	std::cout << "\n";
	while (!g_interrupt) {
		if (!user_line(fit(" ", margin) + "Select: ", choice, true))
			return 0;
		if (choice <= 0 || static_cast<size_t>(choice) > menu.size() + 1) {
			// eraseLines(1);
			std::cout << fit(" ", margin) << "Choice not available... ";
		} else {
			// eraseLines(menu.size() + 2);
			std::cout << fit(" ", margin) << "* " << menu[choice - 1] << "\n\n";
			break;
		}
	};
	return static_cast<size_t>(choice);
}

void Minitel::png_to_mosaique(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) return;

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    png_infop info = png_create_info_struct(png);
    png_init_io(png, fp);
    png_read_info(png, info);

    int width  = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);
    int color_type = png_get_color_type(png, info);
    int bit_depth  = png_get_bit_depth(png, info);

    if (bit_depth == 16) png_set_strip_16(png);
    if (color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_RGB_ALPHA)
        png_set_rgb_to_gray_fixed(png, 1, -1, -1);
    if (color_type & PNG_COLOR_MASK_ALPHA)
        png_set_strip_alpha(png);

    png_read_update_info(png, info);

    std::vector<unsigned char> image(width * height);
    std::vector<png_bytep> rows(height);

    for (int y = 0; y < height; y++)
        rows[y] = &image[y * width];

    png_read_image(png, rows.data());

    fclose(fp);
    png_destroy_read_struct(&png, &info, nullptr);

	cursor_to(1, cursorY);
    // Enter mosaic mode
    send(SO);
    for (int y = 0; y + 2 < height; y += 3) {
        for (int x = 0; x + 1 < width; x += 2) {

            unsigned char bits = 0;

            auto pix = [&](int dx, int dy, int bit) {
                if (image[(y + dy) * width + (x + dx)] < 128)
                    bits |= (1 << bit);
            };

            pix(0, 0, 0);
            pix(1, 0, 1);
            pix(0, 1, 2);
            pix(1, 1, 3);
            pix(0, 2, 4);
            pix(1, 2, 5);

            unsigned char c = 0x20 + bits;
            send(std::string(1, c));
			if (++cursorX > 40) {
				cursorX = 1;
				if (cursorY < ROWS_VIDEOTEX) cursorY++;
			}
			
        }
    }
    send(SI);

	tcdrain(_serial_port);
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
	if (align == EditionMode::LEFT)
		std::cout << "cursor x: " << cursorX << std::endl;
	for (size_t i = 0; i < text.size(); i++) {
		// handle right margin first -> finish on newline
		if (cursorX > colEnd) {
			for (int i = 0; i < margin; i++) 
				writeByte(' ');
			if (cursorY < ROWS_VIDEOTEX) 
				cursorY++;
			cursorX = 1;
		}

		// If the word is too long for the space left, write it on new line (if its longer than the width)
		if (align == EditionMode::LEFT && cursorX + len - 1 > colEnd && len <= lineWidth) {
			for (int i = cursorX; i <= COLS_VIDEOTEX; i++) writeByte(' ');
			if (cursorY < ROWS_VIDEOTEX) 
				cursorY++;
			cursorX = 1;
		}

		// finally handle left margin
		if (cursorX < colStart) {
			send(get_typo());
			for (int i = 0; i < margin; i++) writeByte(' ');
			cursorX = colStart;
			while (align == EditionMode::LEFT && text[i] == ' ') i++;
		}
		writeByte(text[i]);
		if (text[i] == '\r') {
			cursorX = 1; 
		} else if (text[i] == '\n') { 
			send(get_typo()); 
			if (cursorY < ROWS_VIDEOTEX) cursorY++;
		} else {
			cursorX++;
			if (text[i] == ' ')
        		len = nextWordLength(text, i + 1);
		}
	}
	if (cursorX > colEnd) {
		for (int i = 0; i < margin; i++) writeByte(' ');
		cursorX = 1;
		if (cursorY < ROWS_VIDEOTEX) cursorY++;
	}

	// std::cout << std::dec << "write_text: cursorX: " << cursorX << " cursorY: " << cursorY << "\n";
}

void Minitel::cursor_to(int col, int row)
{
	if (row > ROWS_VIDEOTEX || row < 1)
		return;
	if (col > COLS_VIDEOTEX || col < 1)
		return;

    char cmd[3];

	cmd[0] = 0x1F;
	cmd[1] = (char)(row + 0x40);
	cmd[2] = (char)(col + 0x40);

	cursorX = col;
	cursorY = row;
	writeByte(cmd[0]);
	writeByte(cmd[1]);
	writeByte(cmd[2]);
	tcdrain(_serial_port);
}

void Minitel::send_file(const std::string &path, size_t lines) {
	std::ifstream ifile(path, std::ios::binary);
    if (!ifile.is_open()) {
        std::cerr << "Error opening file\n";
        return;
    }

    std::string line;
    write_text("\r", 0);

    if (lines == 0) {
        while (std::getline(ifile, line)) {
			if (!line.empty() && line.back() == '\r')
        		line.pop_back();
            write_text(line);
		}
    } else {
        std::deque<std::string> buffer;
        while (std::getline(ifile, line)) {
            buffer.push_back(line);
            if (buffer.size() > lines)
                buffer.pop_front();
        }
        for (std::string &l : buffer) {
			if (!l.empty() && l.back() == '\r')
        		l.pop_back();
            write_text(l);
		}
    }
    ifile.close();
}

void Minitel::init_tph(std::string& path) {
	try {
		_printer = new ThermalPrinter(path);
		std::cout << "printer initialized\n";
	} catch (std::exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
	}
}

void Minitel::writeByte(unsigned char b) {
    // If using hardware 7E1, remove parity calc here
    // If keeping CS8 software parity, keep it

    ssize_t written;
    int retry = 0;
    int delay_us = 1000; // Start at 1ms
    
    do {
        written = ::write(_serial_port, &b, 1);
        if (written == -1 && errno == EAGAIN) {
            usleep(delay_us);
            delay_us = std::min(delay_us * 2, 50000); // Cap at 50ms
            retry++;
        }
		tcdrain(_serial_port); // force flush after EVERY byte
        usleep(9200);          // 9.2ms = exactly 1 byte at 1200 baud 7E1
    } while (written == -1 && errno == EAGAIN && retry < 50); // Much higher retry
    
    if (written != 1) {
        std::cerr << "Write FAILED after " << retry 
                  << " retries: " << strerror(errno) << "\n";
    }
}

// void Minitel::writeByte(unsigned char b) {
//     // Calculate even parity for lower 7 bits
//     // bool parity = __builtin_parity(b & 0x7F);
//
//     // Set parity bit (bit 7)
//     // if (parity) {
//     //     b |= 0x80;
//     // } else {
//     //     b &= 0x7F;
//     // }
// 	b &= 0x7F; // Safety: mask to 7 bits, hardware appends parity
// 	ssize_t written;
// 	int retry = 0;
// 	do {
// 		written = ::write(_serial_port, &b, 1);
// 		if (written == -1 && errno == EAGAIN) {
// 			usleep(1000); // wait 1ms and retry
// 			retry++;
// 		}
// 	} while (written == -1 && errno == EAGAIN && retry < 10);
// 	if (written == -1) {
// 		if (errno == EIO || errno == ENXIO) {
// 			std::cerr << "Device disconnected!\n";
// 			::close(_serial_port);
// 			_serial_port = -1;
// 			// optionally try to reconnect
// 		}
// 		std::cerr << "Write failed: " << strerror(errno) << "\n";
// 	}
// }

// Does not update cursor -- keep for command
void Minitel::send(const std::string& text) {
	if (text.empty())
		return;
	
	for (size_t i = 0; i < text.length(); i++) {
        writeByte(text[i]);
    }
    tcdrain(_serial_port);
}

void Minitel::close() {
    if (_serial_port < 0) return;
    
    std::cout << "Close.\n";
    // Restore terminal settings before closing
    if (tcgetattr(_serial_port, &original_termios_) == 0) {
        tcsetattr(_serial_port, TCSANOW, &original_termios_);
    }
    
    tcflush(_serial_port, TCIOFLUSH);
    ::close(_serial_port);
    _serial_port = -1;  // ✓ Guard against double close
}

Minitel::State Minitel::redirect_input(State state, const std::string& input) {
	send(COFF);
	std::cout << "-> Redirect user input to " << get_state(state) << "\n";
	switch (state) {
		case Minitel::State::MENU:
			state = index->handle_input(input); break;
		case Minitel::State::HAZARDOUS:
			state = maze->handle_input(input); break;
		case Minitel::State::STORY:
			state = cadavre->handle_input(input); break;
		case Minitel::State::EMAIL:
			state = contact->handle_input(input); break;
		case Minitel::State::FORTY2:
			state = forty2->handle_input(input); break;
		default:
			return Minitel::State::MENU;
	}
	return state;
}

Minitel::State Minitel::redirect_display(State state, bool waiting) {
	std::cout << "-> User get redirected.\n";
	std::cout << "\tfrom   : " << get_state(_state) << "\n";
	std::cout << "\tto     : " << get_state(state) << "\n";

	if (waiting) {
		update_cursor(COLS_VIDEOTEX / 2 - 11, ROWS_VIDEOTEX / 2, 0);
		write_text(" -> redirect in ");
		for (int i = 3; i > 0; i--) {
			write_text(std::to_string(i) + " sec ");
			cursor_to(COLS_VIDEOTEX / 2 - 11 + 16, ROWS_VIDEOTEX / 2);
			sleep(1);
		}
	}

	switch (state) {
		case Minitel::State::MENU:
			index->display();
			break;
		case Minitel::State::HAZARDOUS:
			maze->display();
			break;
		case Minitel::State::STORY:
			cadavre->display();
			break;
		case Minitel::State::EMAIL:
			contact->display();
			break;
		case Minitel::State::FORTY2:
			forty2->display();
			break;
		default:
			return Minitel::State::MENU;
	}
	send(CON);
	return state;
}

void Minitel::ascii_noise(int amount) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> rX(1, COLS_VIDEOTEX);
	std::uniform_int_distribution<int> rY(1, ROWS_VIDEOTEX);
	std::uniform_int_distribution<unsigned char> rChar(32, 127);
	// send(COFF);
	for (int i = 0; i < amount; i++) {
		update_cursor(rX(gen), rY(gen), 0);
		writeByte(rChar(gen));
	}
	update_cursor(rX(gen), rY(gen), 0);
	// send(CON);
}

ThermalPrinter* Minitel::get_printer() {return _printer;}
