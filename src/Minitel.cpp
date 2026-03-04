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



Minitel::Minitel(void): 
	_paper(BLACK_BCKG),
	_ink(WHITE_CHAR),
	_marginX(1),
	_cursorX(1),
	_cursorY(1),
	_bufind(0),
	_serial_port(-1),
	_mode(VIDEOTEX),
	_state(State::MENU),
	_rbuff(""),
	_buffer(""),
	_story(""),
	_debugMode(false) {}

Minitel::~Minitel(void) {
	this->close();
	if (_storyBook.is_open()) {
		_storyBook.close();
		std::cout << "Close story book.\n";
	}
	if (_contacts.is_open()) {
		_storyBook.close();
		std::cout << "Close story book.\n";
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

	send(INIT);
	std::cout << "Minitel initialized with sequence: " << INIT << "\n";
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
	// NONBLOCK because of poll (taht allready block for incoming data)
    _serial_port = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (_serial_port == -1) {
        perror("Erreur ouverture port");
        return -1;
    }
	usleep(1000000); // 100ms — let USB enumerate properly
	tcgetattr(_serial_port, &original_termios_);

    tcflush(_serial_port, TCIOFLUSH);  // Flush both input and output
    // Remove O_NONBLOCK after opening
    // int flags = fcntl(serial_port_, F_GETFL, 0);
    // fcntl(serial_port_, F_SETFL, flags & ~O_NONBLOCK);
    
    // Flush any existing data BEFORE configuring
    tcflush(_serial_port, TCIOFLUSH);
    
    // Get current settings and clear them completely
    struct termios options;
	if (tcgetattr(_serial_port, &options) != 0) {
        perror("tcgetattr");
        return -1;
    }
    
    // Clear all flags to start fresh
    cfmakeraw(&options);  // Sets up raw mode cleanly
    cfsetospeed(&options, B1200);   // Output speed ← ADD THIS!
    // Set baud rate
    cfsetispeed(&options, B1200);
    // CS8, no parity - macOS compatible
	options.c_cflag &= ~CSIZE;    // Clear size bits
	options.c_cflag |= CS8;       // 8 bits (macOS compatible)
	options.c_cflag &= ~PARENB;   // No hardware parity
	options.c_cflag &= ~PARODD;   // (just in case)
	options.c_cflag &= ~CSTOPB;   // 1 stop bit
	options.c_cflag &= ~CRTSCTS;  // No flow control
	options.c_cflag |= CREAD;     // Enable receiver
	options.c_cflag |= CLOCAL;    // Ignore modem lines

	options.c_iflag  = 0;         // No input processing
	options.c_oflag  = 0;         // No output processing
	options.c_lflag  = 0;         // Raw mode

	options.c_cc[VMIN]  = 0;
	options.c_cc[VTIME] = 0;
    // 7E1 configuration
	// options.c_cflag &= ~CSIZE;   // ← Clear size bits
	// options.c_cflag |= CS7;      // ← Then set CS7
	// options.c_cflag |= PARENB;   // ← Then set parity
	// options.c_cflag &= ~PARODD;  // ← Even parity
	// options.c_cflag &= ~CSTOPB;  // ← 1 stop bit
	// options.c_cflag &= ~CRTSCTS; // ← No flow control
	// options.c_cflag |= CREAD;    // ← Enable receiver
	// options.c_cflag |= CLOCAL;   // ← Ignore modem lines

    // Apply settings with TCSAFLUSH to discard data
    tcsetattr(_serial_port, TCSAFLUSH, &options);
    
    // Flush again after configuration
    tcflush(_serial_port, TCIOFLUSH);
    
    // DTR/RTS control - toggle to reset Minitel state
    int status;
    ioctl(_serial_port, TIOCMGET, &status);
    status |= TIOCM_DTR | TIOCM_RTS;
    ioctl(_serial_port, TIOCMSET, &status);
    
    // Wait for Minitel to be ready
    usleep(500000);  // 500ms

	// STEP 3: Read and discard anything that arrived
    char discard_buf[256];
    int discarded = 0;
    int n;
    while ((n = ::read(_serial_port, discard_buf, sizeof(discard_buf))) > 0) {
        discarded += n;
        std::cout << "Discarding " << n << " startup bytes: ";
        for (int i = 0; i < n; i++) {
            printf("%02X ", (unsigned char)discard_buf[i]);
        }
        std::cout << "\n";
        usleep(80000);  // Small delay between reads
    }
    std::cout << "Discarded " << discarded << " startup bytes total\n";
    
	struct termios current;
	tcgetattr(_serial_port, &current);

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
			eraseLines(1);
			std::cout << fit(" ", margin) << "Choice not available... ";
		} else {
			eraseLines(menu.size() + 2);
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
        }
    }

	tcdrain(_serial_port);
    send(SI);

	    // Calculate final cursor position
    int chars_wide = width / 2;
    int chars_tall = height / 3;
    
    // Update tracked position
    _cursorX = _cursorX + chars_wide;  // Moved right by image width
    _cursorY = _cursorY + chars_tall;  // Moved down by image height
    
    // Handle line overflow
    if (_cursorX > 40) {
        _cursorY += _cursorX / 40;
        _cursorX = _cursorY % 40;
    }
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

	_cursorX = col;
	_cursorY = row;
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

void Minitel::writeByte(unsigned char b) {
    // Calculate even parity for lower 7 bits
    bool parity = __builtin_parity(b & 0x7F);

    // Set parity bit (bit 7)
    if (parity) {
        b |= 0x80;
    } else {
        b &= 0x7F;
    }
	ssize_t written;
	int retry = 0;
	do {
		written = ::write(_serial_port, &b, 1);
		if (written == -1 && errno == EAGAIN) {
			usleep(1000); // wait 1ms and retry
			retry++;
		}
	} while (written == -1 && errno == EAGAIN && retry < 10);
	if (written == -1) {
		if (errno == EIO || errno == ENXIO) {
			std::cerr << "Device disconnected!\n";
			::close(_serial_port);
			_serial_port = -1;
			// optionally try to reconnect
		}
		std::cerr << "Write failed: " << strerror(errno) << "\n";
	}
	//    ssize_t written = ::write(_serial_port, &b, 1);
	// if (written != 1) {
	// 	std::cerr << "Write failed! errno: " << errno 
	// 			  << " (" << strerror(errno) << ")\n";
	// 	// Handle error - maybe reconnect?
	// 	return;
	// }
	// 	    // CRITICAL: Wait for data to actually be transmitted
	//    if (tcdrain(_serial_port) != 0) {
	//        std::cerr << "tcdrain failed! errno: " << errno 
	//                  << " (" << strerror(errno) << ")\n";
	//        // TX has stopped!
	//    }
}

// Does not update cursor -- keep for command
void Minitel::send(const std::string& text) {
	if (text.empty())
		return;
	const size_t CHUNK_SIZE = 32;  // Send in small chunks
	
	for (size_t pos = 0; pos < text.length(); pos += CHUNK_SIZE) {
        size_t chunk_len = std::min(CHUNK_SIZE, text.length() - pos);

		for (size_t i = 0; i < chunk_len; i++) {
			writeByte(text[pos + i]);
		}
		// Wait for chunk to be sent before continuing
        tcdrain(_serial_port);
        
        // Small delay between chunks
        usleep(10000);  // 10ms
	}
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
