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

#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOCFPlugIn.h>


Minitel::Minitel(void): 
	serial_port_(-1), mode_(VIDEOTEX), state_(State::MENU), buffer_(""), story_(""), debugMode_(false) {}

Minitel::~Minitel(void) {
	this->close();
	if (storyBook_.is_open()) {
		storyBook_.close();
		std::cout << "Close story book.\n";
	}
}

int Minitel::init(int ac, char** av) {
	if (ac > 1) {
		if (std::string(av[1]) == "-d") {
			debugMode_ = true;
		}
	}
	std::cout << "Opening story book.\n";
	storyBook_.open("cadavre.txt");
	if (!storyBook_.is_open()) {
		std::cerr << "Error opening story book.\n";
		return -1;
	}
	return 0;
}

int Minitel::configure_serial(const char* port) {
    serial_port_ = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (serial_port_ == -1) {
        perror("Erreur ouverture port");
        return -1;
    }
    
    // Remove O_NONBLOCK after opening
    int flags = fcntl(serial_port_, F_GETFL, 0);
    fcntl(serial_port_, F_SETFL, flags & ~O_NONBLOCK);
    
    // Flush any existing data BEFORE configuring
    tcflush(serial_port_, TCIOFLUSH);
    
    // Get current settings and clear them completely
    struct termios options;
    tcgetattr(serial_port_, &options);  // Get current first
    
    // Clear all flags to start fresh
    cfmakeraw(&options);  // Sets up raw mode cleanly
    
    // Set baud rate
    cfsetispeed(&options, B1200);
    cfsetospeed(&options, B1200);
    
    // 7E1 configuration
    options.c_cflag = CS7 | CLOCAL | CREAD | PARENB;  // Build from scratch
    options.c_cflag &= ~(PARODD | CSTOPB | CRTSCTS);
    
    // Input flags
    options.c_iflag = INPCK;  // Enable parity checking
    options.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL | INLCR | IGNCR | IGNBRK | BRKINT | PARMRK | ISTRIP);
    
    // Output flags - completely raw
    options.c_oflag = 0;
    
    // Local flags - completely raw
    options.c_lflag = 0;
    
    // Control characters
    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 10;  // 1 second timeout
    
    // Apply settings with TCSAFLUSH to discard data
    tcsetattr(serial_port_, TCSAFLUSH, &options);
    
    // Flush again after configuration
    tcflush(serial_port_, TCIOFLUSH);
    
    // DTR/RTS control - toggle to reset Minitel state
    int status;
    ioctl(serial_port_, TIOCMGET, &status);
    status |= TIOCM_DTR | TIOCM_RTS;
    ioctl(serial_port_, TIOCMSET, &status);
    
    // Wait for Minitel to be ready
    usleep(500000);  // 500ms - give more time
    
    return 1;
}

// int Minitel::configure_serial(const char* port) {
//     serial_port_ = open(port, O_RDWR | O_NOCTTY);
//     if (serial_port_ == -1) {
//         perror("Erreur ouverture port");
//         return -1;
//     }
//     tcflush(serial_port_, TCIOFLUSH);
//     struct termios options;
//     memset(&options, 0, sizeof(options));
//
//     // 1200 baud
//     cfsetispeed(&options, B1200);
//     cfsetospeed(&options, B1200);
//
//     // 7E1
//     options.c_cflag |= CLOCAL | CREAD;
//     options.c_cflag |= CS7;
//     options.c_cflag |= PARENB;   // Even parity
//     options.c_cflag &= ~PARODD;
//     options.c_cflag &= ~CSTOPB;  // 1 stop bit
//
//     // No flow control
//     options.c_iflag &= ~(IXON | IXOFF | IXANY);
//     options.c_cflag &= ~CRTSCTS;
//
//     // Raw mode
//     options.c_iflag &= ~(ICRNL | INLCR | IGNCR);
//     options.c_lflag = 0;
//     options.c_oflag = 0;
//
//     // Read timeout
//     options.c_cc[VMIN]  = 0;
//     options.c_cc[VTIME] = 1;  // 100 ms
//
//     tcsetattr(serial_port_, TCSANOW, &options);
//     tcflush(serial_port_, TCIOFLUSH);
//
//     usleep(300000);
//     return 1;
// }
//
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

void Minitel::png_to_mosaique_dither(const char* filename) {
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
    
    // Use signed int for dithering error propagation
    std::vector<int> image(width * height);
    std::vector<png_bytep> rows(height);
    std::vector<unsigned char> temp(width * height);
    
    for (int y = 0; y < height; y++)
        rows[y] = &temp[y * width];
    
    png_read_image(png, rows.data());
    fclose(fp);
    png_destroy_read_struct(&png, &info, nullptr);
    
    // Convert to signed int for dithering
    for (size_t i = 0; i < temp.size(); i++)
        image[i] = temp[i];
    
    // Floyd-Steinberg dithering with clamping
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            int old_pixel = std::max(0, std::min(255, image[idx]));
            int new_pixel = (old_pixel < 128) ? 0 : 255;
            image[idx] = new_pixel;
            
            int error = old_pixel - new_pixel;
            
            // Helper to add error with clamping
            auto add_error = [&](int offset, int factor) {
                int val = image[offset] + error * factor / 16;
                image[offset] = std::max(0, std::min(255, val));
            };
            
            // Distribute error to neighboring pixels
            if (x + 1 < width)
                add_error(idx + 1, 7);
            if (y + 1 < height) {
                if (x > 0)
                    add_error(idx + width - 1, 3);
                add_error(idx + width, 5);
                if (x + 1 < width)
                    add_error(idx + width + 1, 1);
            }
        }
    }
    
	try {
		// Enter mosaic mode
		write(serial_port_, "\x0E", 1);
		tcdrain(serial_port_);
		usleep(50000);
		
		for (int y = 0; y + 2 < height; y += 3) {
			for (int x = 0; x + 1 < width; x += 2) {
				unsigned char bits = 0;
				
				auto pix = [&](int dx, int dy, int bit) {
					int px = x + dx;
					int py = y + dy;
					if (px >= width || py >= height) return;  // Bounds check
					int val = image[py * width + px];
					if (val < 128)
						bits |= (1 << bit);
				};
				
				pix(0, 0, 0);
				pix(1, 0, 1);
				pix(0, 1, 2);
				pix(1, 1, 3);
				pix(0, 2, 4);
				pix(1, 2, 5);
				
				unsigned char c = 0x20 + bits;
				write(serial_port_, &c, 1);
				usleep(2000);  // Increased delay
			}
			write(serial_port_, "\r", 1);  // Add newline
			usleep(30000);  // Increased delay between rows
		}
	} catch (std::exception& e) {
		std::cout << "Error: " << e.what() << "\n";	
    }
    tcdrain(serial_port_);
    write(serial_port_, "\x0F", 1);  // Exit mosaic
    tcdrain(serial_port_);
    usleep(50000);

    // Back to alphanumeric
    cursor_at(13, 1);
}

void Minitel::recovery() {
    std::cerr << "Starting Minitel recovery...\n";
    
    // 1. Flush all buffers (kernel side)
    tcflush(serial_port_, TCIOFLUSH);
    usleep(100000);
    
    // 2. Send break signal (low-level reset)
    tcsendbreak(serial_port_, 0);
    usleep(200000);
    
    // 3. Reconfigure serial port
    struct termios tty;
    tcgetattr(serial_port_, &tty);
    cfsetospeed(&tty, B4800);
    cfsetispeed(&tty, B4800);
    tcsetattr(serial_port_, TCSANOW, &tty);
    
    // 4. Send escape sequences
    const char* recovery_cmds[] = {
        "\x18",          // CAN - Cancel current operation
        "\x0F",          // SI - Exit mosaic mode
        "\x11",          // DC1 - Exit attributes
        "\x0C",          // FF - Clear screen
        "\x1B\x3A\x69\x45",  // Protocol reset
        "\x1E"           // RS - Home cursor
    };
    
    for (const char* cmd : recovery_cmds) {
        write(serial_port_, cmd, strlen(cmd));
        tcdrain(serial_port_);
        usleep(100000);
    }
    
    std::cerr << "Recovery complete\n";
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
    write(serial_port_, "\x0E", 1);

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
            write(serial_port_, &c, 1);
			usleep(1000);
        }
		write(serial_port_, "\r", 1);  // ← IMPORTANT
    	usleep(20000);
    }

	tcdrain(serial_port_);
    write(serial_port_, "\x0F", 1);
}

void Minitel::cursor_at(int row, int col) {
	if (row > ROWS_VIDEOTEX || row <= 0)
		return;
	if (col > COLS_VIDEOTEX || col <= 0)
		return;

    char cmd[3];

	cmd[0] = 0x1F;
	cmd[1] = (char)(row + 0x40);
	cmd[2] = (char)(col + 0x40);

	write(serial_port_, cmd, 3);
	tcdrain(serial_port_);
}

void Minitel::bell() {
    write(serial_port_, "\x07", 1);
	tcdrain(serial_port_);
}

void Minitel::clear_line() {
	unsigned char c = 0x18;
	write(serial_port_, &c, 1);
	tcdrain(serial_port_);
}

void Minitel::roll_mode() {
	char cmd[3];

	cmd[0] = 0x1B;
	cmd[1] = 0x6A;
	cmd[2] = 0x43;

	write(serial_port_, &cmd, 3);
	tcdrain(serial_port_);
}

void Minitel::mosaique_mode() {
    write(serial_port_, "\x0E", 1);
	tcdrain(serial_port_);
}

void Minitel::ascii_mode() {
    write(serial_port_, "\x0F", 1);
}

void Minitel::g2_mode() {
    write(serial_port_, "\x19", 1);
}

void Minitel::blink_on() {
	write(serial_port_, "\x1B\x48", 2);
}

void Minitel::blink_off() {
	write(serial_port_, "\x1B\x49", 2);
}

void Minitel::test_char() {
	std::string str = " Hazardous color test\r\n";

	char characters[2];
	characters[0] = 0x1B;
	characters[1] = 0x40;

	char background[2];
	background[0] = 0x1B;
	background[1] = 0x50;

	for (int i = 0; i < 8; i++) {
		write(serial_port_, characters, 2);
		tcdrain(serial_port_);
		usleep(10000);  // Wait for command to process
		for (int j = 0; j < 8; j++) {
			write(serial_port_, background, 2);
			tcdrain(serial_port_);
			usleep(10000);  // Wait for command to process
			send(str);
			background[1] = background[1] + 1;
		}
		send("\r\n");
		characters[1] = characters[1] + 1;
	}
}

void Minitel::display_ascii_table() {
    for (int i = 0x20; i <= 0x7F; i++) {   // Minitel mosaic range
        unsigned char c = i;
        write(serial_port_, &c, 1);
        // New line every 8 characters
        if ((i - 0x20 + 1) % 8 == 0) {
            write(serial_port_, "\r\n", 2);
        }
    }
    // Ensure we end in alphanumeric mode
}

void Minitel::cursor_on() {
	unsigned char c = 0x11;
	write(serial_port_, &c, 1);
}

void Minitel::cursor_off() {
	unsigned char c = 0x14;
	write(serial_port_, &c, 1);
}

void Minitel::send(const std::string& text) {
	if (text.empty())
		return;
    write(serial_port_, text.c_str(), text.length());
    usleep(300000); // Petit délai pour ne pas surcharger
}

void Minitel::clear_screen() {
	if (mode_ == VIDEOTEX) {
		write(serial_port_, "\x1B\x0C", 2);
	} else if (mode_ == ANSI) {
		write(serial_port_, "\x1B[2J\x1B[H", 7);
	}
    usleep(300000);
}

void Minitel::close() {
    if (serial_port_ <= 0) return;
    
	std::cout << "Close.\n";
    // Flush all buffers (discard pending data)
    tcflush(serial_port_, TCIOFLUSH);
    
    // Close immediately
    ::close(serial_port_);
    serial_port_ = -1;
}

void Minitel::fsend(
    const std::string& line,
    const std::string& cmd1,
    const std::string& cmd2,
    const std::string& cmd3)
{
    std::istringstream iss(line);
    std::string word;
    std::string fline;

    while (iss >> word) {
        // +1 for space if fline is not empty
        if (fline.length() + word.length() + (fline.empty() ? 0 : 1) <= 40) {
            if (!fline.empty())
                fline += " ";
            fline += word;
        } else {
            // send current line
            send(cmd1);
            send(cmd2);
            send(cmd3);
            send(fline);

			send("\r\n");
            // start new line
            fline = word;
        }
    }

    // send remaining line
    if (!fline.empty()) {
        send(cmd1);
        send(cmd2);
        send(cmd3);
        send(fline);
    }
}

