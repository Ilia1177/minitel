#include "ThermalPrinter.hpp"
#include "hzdios.hpp"
#include <vector>
#include <png.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <cerrno>
#include <iostream>
#include <termios.h>  // For tcdrain()
#include <random>

std::vector<unsigned char> CMD_HT      			= {0x09};              // HT
std::vector<unsigned char> CMD_LF      			= {0x0A};              // LF
std::vector<unsigned char> CMD_CR      			= {0x0D};              // CR
std::vector<unsigned char> CMD_INIT    			= {0x1B, 0x40};        // ESC @

std::vector<unsigned char> CMD_FEED_DOTS		= {0x1B, 0x4A, 0x00};   // ESC J n
std::vector<unsigned char> CMD_FEED_LINES		= {0x1B, 0x64, 0x01};  // ESC d n

std::vector<unsigned char> CMD_LINE_SPACE_DEFAULT = {0x1B, 0x32};      // ESC 2
std::vector<unsigned char> CMD_LINE_SPACE_SET     = {0x1B, 0x33, 0x00}; // ESC 3 n

std::vector<unsigned char> CMD_ALIGN_LEFT   = {0x1B, 0x61, 0x00}; // ESC a 0
std::vector<unsigned char> CMD_ALIGN_CENTER = {0x1B, 0x61, 0x01}; // ESC a 1
std::vector<unsigned char> CMD_ALIGN_RIGHT  = {0x1B, 0x61, 0x02}; // ESC a 2

std::vector<unsigned char> CMD_ABS_POS = {0x1B, 0x24, 0x00, 0x00}; // ESC $ nL nH

std::vector<unsigned char> CMD_FONT_A = {0x1B, 0x4D, 0x00}; // ESC M 0
std::vector<unsigned char> CMD_FONT_B = {0x1B, 0x4D, 0x01}; // ESC M 1

std::vector<unsigned char> CMD_BOLD_ON  = {0x1B, 0x45, 0x01}; // ESC E 1
std::vector<unsigned char> CMD_BOLD_OFF = {0x1B, 0x45, 0x00}; // ESC E 0

std::vector<unsigned char> CMD_DOUBLE_STRIKE_ON  = {0x1B, 0x47, 0x01}; // ESC G 1
std::vector<unsigned char> CMD_DOUBLE_STRIKE_OFF = {0x1B, 0x47, 0x00}; // ESC G 0

std::vector<unsigned char> CMD_ROTATE_ON  = {0x1B, 0x56, 0x01}; // ESC V 1
std::vector<unsigned char> CMD_ROTATE_OFF = {0x1B, 0x56, 0x00}; // ESC V 0

std::vector<unsigned char> CMD_UNDERLINE_OFF = {0x1B, 0x2D, 0x00}; // ESC - 0
std::vector<unsigned char> CMD_UNDERLINE_1   = {0x1B, 0x2D, 0x01}; // ESC - 1
std::vector<unsigned char> CMD_UNDERLINE_2   = {0x1B, 0x2D, 0x02}; // ESC - 2

std::vector<unsigned char> CMD_CHAR_SPACING = {0x1B, 0x20, 0x00}; // ESC SP n

std::vector<unsigned char> CMD_BIT_IMAGE   = {0x1B, 0x2A}; // ESC *
std::vector<unsigned char> CMD_DL_IMAGE    = {0x1D, 0x2A}; // GS *
std::vector<unsigned char> CMD_PRINT_DLIMG = {0x1D, 0x2F}; // GS /
std::vector<unsigned char> CMD_PRINT_LINE  = {0x1D, 0x22}; // GS "
														   //
std::vector<unsigned char> CMD_BAR_HRI_POS = {0x1D, 0x48, 0x00}; // GS H n
std::vector<unsigned char> CMD_BAR_HEIGHT  = {0x1D, 0x68, 0x00}; // GS h n
std::vector<unsigned char> CMD_BAR_WIDTH   = {0x1D, 0x77, 0x00}; // GS w n
std::vector<unsigned char> CMD_BAR_FONT    = {0x1D, 0x66, 0x00}; // GS f n
std::vector<unsigned char> CMD_BAR_PRINT   = {0x1D, 0x6B};      // GS k


bool isPrinterReady(int fd) {
    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(fd, &writefds);
    
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;  // Immediate return
    
    int ready = select(fd + 1, NULL, &writefds, NULL, &timeout);
    
    return (ready > 0);  // true if ready to accept more data
}

// Default constructor
ThermalPrinter::ThermalPrinter(const std::string& path): 
	_devicePath(path), _current(nullptr) {

	if (!open_device(path)) {
		throw std::runtime_error("Cannot open device.");
	}
	_fontA = true;
	// Initialize printer
	std::vector<unsigned char> init = {0x1B, 0x40};
	write(_fd, init.data(), init.size());
	usleep(50000);
}

// Destructor
ThermalPrinter::~ThermalPrinter(void) {
	if (_fd >= 0) {
		::close(_fd);
	}
	if (_current) {
		hzdpng_free(_current);
	}
}

ThermalPrinter::ThermalPrinter(void):
	_devicePath(""), _fd(-1), _current(nullptr) {
	return;
}

bool ThermalPrinter::init(const std::string& path) {
	if (!open_device(path))
		return false;
	writeCommand(CMD_INIT);
	std::cout << "Printer initialized !\n";
	return true;
}

void ThermalPrinter::align(AlignDef pos) {
	if (pos == LEFT) {
		writeCommand(CMD_ALIGN_LEFT);
	} else if (pos == RIGHT) {
		writeCommand(CMD_ALIGN_RIGHT);
	} else if (pos == CENTER) {
		writeCommand(CMD_ALIGN_CENTER);
	}
}

void ThermalPrinter::close_device() {
	    if (_fd < 0)
			return;
        std::vector<unsigned char> feed = {0x1B, 0x64, 0x05};  // Feed 5 lines
        writeCommand(feed);
        usleep(300000);  // Wait 300ms
						 //
        std::cout << "Closing printer gracefully..." << std::endl;

        tcflush(_fd, TCOFLUSH);
		reset();
        // 1. Drain output buffer (wait for transmission)
        tcdrain(_fd);
        
        // 2. Feed some paper to finish cleanly

		struct termios tio;
		if (tcgetattr(_fd, &tio) == 0) {
			cfmakeraw(&tio);
			tcsetattr(_fd, TCSANOW, &tio);
		}
        // 3. Reset printer to defaults
        usleep(100000);
        
        // 4. Close file descriptor
        ::close(_fd);
        _fd = -1;
        
        std::cout << "Printer closed" << std::endl;
}

bool ThermalPrinter::is_open() {
	if (_fd < 0) {
		return false;
	}
	return true;
}

void ThermalPrinter::reset() {
    // ESC @ (0x1B 0x40) - Initialize printer
    std::vector<unsigned char> cmd = {0x1B, 0x40};
    writeCommand(cmd);
    usleep(50000);  // Wait 50ms for reset to complete
}

void ThermalPrinter::writeCommand(const std::vector<unsigned char>& cmd, int timeout) {
	(void)timeout;
	if (_fd < 0) return;
	write(_fd, cmd.data(), cmd.size());
}

void ThermalPrinter::writeText(const std::string& str, int timeout) {
	(void)timeout;
	if (_fd < 0) return;
	write(_fd, str.data(), str.length());
}

bool ThermalPrinter::open_device(const std::string& path) {
    _fd = ::open(path.c_str(), O_WRONLY | O_NOCTTY);
    if (_fd < 0) {
        perror("open printer");
        return false;
    }
    return true;
}

// USA
// France
// Germany
// U.K
// Denmark I
// Sweden
// Italy
// Spain I
// Japan
// Norway
// Denmark II
// Spain II
// Latin America
// Korea	
// Slovenia/Croatia
// China
void ThermalPrinter::asciiset(int set) {
	if (set > 15 || set < 0) {
		std::cerr << "Not valid font";
	}
	std::vector<unsigned char> cmd = {0x1B, 0x52, 0x00};
	cmd[2] = set;
	writeCommand(cmd);
}

void ThermalPrinter::toggle_fontAB() {
	_fontA = !_fontA;
	std::vector<unsigned char> cmd = {0x1B, 0x4D, 0x00};
	cmd[2] = _fontA ? 1 : 0;
	writeCommand(cmd);
}

void ThermalPrinter::dot_feed(int dots) {
	std::vector<unsigned char> feed = {0x1B, 0x66, 0x00};
	feed[2] = static_cast<unsigned char>(dots);
	writeCommand(feed);
}

void ThermalPrinter::feed(int lines) {
	std::vector<unsigned char> feed = {0x1B, 0x64, 0x5};
	feed[2] = static_cast<unsigned char>(lines);
	writeCommand(feed);
}

void ThermalPrinter::printPNG_chunked(const std::vector<uint8_t>& bitImage,
                           int width, int height,
                           int chunkHeight) 
{
    int widthBytes = (width + 7) / 8;

    for (int y0 = 0; y0 < height; y0 += chunkHeight) {
        int bandHeight = std::min(chunkHeight, height - y0);

        // GS v 0 header
        std::vector<uint8_t> header = {
            0x1D, 0x76, 0x30, 0x00,
            static_cast<uint8_t>(widthBytes & 0xFF),
            static_cast<uint8_t>((widthBytes >> 8) & 0xFF),
            static_cast<uint8_t>(bandHeight & 0xFF),
            static_cast<uint8_t>((bandHeight >> 8) & 0xFF)
        };

        writeCommand(header);

        // Send the data for this chunk
        const uint8_t* ptr = &bitImage[y0 * widthBytes];
        writeCommand(std::vector<uint8_t>(ptr, ptr + bandHeight * widthBytes));
    }
}

// Takes PNG transform it to 1-bit image and send to printer
void ThermalPrinter::printPNG(const std::string& filename, screen type) {
	std::cout << "Printing PNG\n";
	writeCommand(CMD_ALIGN_CENTER);

	PNGImage* image = hzdpng_read(filename.c_str());

	if (!image) {
		return;
	} else if (image->width > MAX_PIXEL_WIDTH) {
		std::cout << "resize image\n";
		hzdpng_resize(*image, MAX_PIXEL_WIDTH);
	}

	int width = image->width;
	int height = image->height;
	std::vector<uint8_t> bitImage;
	std::vector<float> grey = hzdpng_grayscale_error(*image);
	hzdpng_free(image);
	int lpi = 0;
	switch (type) {
		case HALFTONE:
			bitImage = hzdpng_halftone_screening(grey, width, height, lpi);
			break;
		case FINGERPRINT:
			bitImage = hzdpng_fingerprint_dither(grey, width, height, 30, 10.2f, 3.2f);
			break;
		case DITHER:
			bitImage = hzdpng_diffusion_dither(grey, width, height);
			break;
		default:
			bitImage = hzdpng_diffusion_dither(grey, width, height);
	}
	printPNG_chunked(bitImage, width, height);
}

int coordinate2d(int x, int y, int w) {

	int index = y * w + x;
	return index;
}

void swap_pixel(unsigned char& a, unsigned char& b) {
	unsigned char tmp = a;

	a = b;
	b = tmp;
}

// unsigned char & random_pixel(PNGImage& img) {
// 	int w = img.width;
// 	int h = img.height;
//     std::random_device dev;
// 	std::mt19937 rng(dev());
// 	std::uniform_int_distribution<std::mt19937::result_type> rX(0,w); // distribution in range [1, 6]
// 	std::uniform_int_distribution<std::mt19937::result_type> rY(0,h); // distribution in range [1, 6]
//
// 	int index = coordinate2d(rX(rng), rY(rng), img.width);
// 	return img.data[index];
// }

// void ThermalPrinter::move_pixel(PNGImage* img) {
// 	if (!img)
// 		return;
//
// 	unsigned char &src = random_pixel(*img);
// 	unsigned char &dest = random_pixel(*img);
//
// 	swap_pixel(src, dest);
// }
//
// #include <functional>
// void ThermalPrinter::destruct(int iteration) {
// 	_current = hzdpng_read("gribouille.png");
// 	if (!_current) return;
//
// 	std::vector<std::reference_wrapper<unsigned char>> blackPixels;
// 	std::vector<std::reference_wrapper<unsigned char>> whitePixels;
//
// 	for (auto& pixel : _current->data) {
// 		if (pixel == 0)
// 			blackPixels.push_back(std::ref(pixel));
// 		else
// 			whitePixels.push_back(std::ref(pixel));
// 	}
//
//
// 	for (int i = 0; i < iteration; i++) {
// 		move_pixel(_current);
// 	}
//
// 	hzdpng_write("result_iteration.png", _current);
//
// }
