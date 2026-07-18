#ifndef THERMALPRINTER_HPP
# define THERMALPRINTER_HPP

# define MAX_PIXEL_WIDTH 384

#include <iostream>
#include "hzdpng.h"

enum AlignDef {
	LEFT,
	RIGHT,
	CENTER
};

class ThermalPrinter
{
  public:
    ~ThermalPrinter();
    ThermalPrinter(void);
    ThermalPrinter(const std::string& devicePath);
	enum screen {DITHER, HALFTONE, FINGERPRINT};

	void toggle_fontAB();
	void asciiset(int set);
	void dot_feed(int lines);
	void feed(int lines);
	void printPNG_chunked(const std::vector<uint8_t>& bitImage, int width, int height, int chunkHeight = 32);
	void printPNG(const std::string& filename, screen type = DITHER);
	void writeCommand(const std::vector<unsigned char>& cmd, int timeout_sec = 5);
	void writeText(const std::string& str, int timeout_sec = 5);
	bool open_device(const std::string& path);
	bool is_open();
	void close_device();

	bool check_connection();
	void reset();
	bool init(const std::string& path);
	void align(AlignDef);
	void destruct(int iteration);
	void move_pixel(PNGImage* img);

  private:
	bool		_fontA;
    std::string _devicePath;
    int         _fd;
	PNGImage*	_current;

    ThermalPrinter(const ThermalPrinter& other);
    ThermalPrinter& operator=(const ThermalPrinter& other);
};

#endif
