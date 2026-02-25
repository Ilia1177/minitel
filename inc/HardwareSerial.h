#include <fstream>
#include <iomanip>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
typedef uint8_t  byte;   // replace Arduino definition
typedef uint16_t word;   // replace Arduino definition

class HardwareSerial {
	public:
		void	write(byte b);
		void	close();
		int 	configure(const char* port);

		int fd;
};
