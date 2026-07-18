#include "HardwareSerial.h"
#include <termios.h>
#include <sys/ioctl.h>

void HardwareSerial::write(uint8_t b) {
    // Calculate even parity for lower 7 bits
    bool parity = __builtin_parity(b & 0x7F);
    
    // Set parity bit (bit 7)
    if (parity) {
        b |= 0x80;   // Set bit 7 to 1
    } else {
        b &= 0x7F;   // Clear bit 7 to 0
    }
    
    ::write(fd, &b, 1);
}

int HardwareSerial::configure(const char* port) {
	// NONBLOCK because of poll (taht allready block for incoming data)
    fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == -1) {
        perror("Erreur ouverture port");
        return -1;
    }
	// tcgetattr(fd, &original_termios_);

    tcflush(fd, TCIOFLUSH);  // Flush both input and output
    // Remove O_NONBLOCK after opening
    // int flags = fcntl(fd, F_GETFL, 0);
    // fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    
    // Flush any existing data BEFORE configuring
    tcflush(fd, TCIOFLUSH);
    
    // Get current settings and clear them completely
    struct termios options;
	if (tcgetattr(fd, &options) != 0) {
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
								  //

	options.c_iflag  = 0;         // No input processing
	options.c_oflag  = 0;         // No output processing
	options.c_lflag  = 0;         // Raw mode

	options.c_cc[VMIN]  = 0;
	options.c_cc[VTIME] = 0;
    // 7E1 configuration
	// options.c_cflag &= ~CSIZE;   // ← Clear size bits first!
	// options.c_cflag |= CS7;      // ← Then set CS7
	// options.c_cflag |= PARENB;   // ← Then set parity
	// options.c_cflag &= ~PARODD;  // ← Even parity
	// options.c_cflag &= ~CSTOPB;  // ← 1 stop bit
	// options.c_cflag &= ~CRTSCTS; // ← No flow control
	// options.c_cflag |= CREAD;    // ← Enable receiver
	// options.c_cflag |= CLOCAL;   // ← Ignore modem lines

	// // Input flags
	// options.c_iflag  = 0;
	// options.c_iflag |= INPCK;    // Enable input parity checking
	//
	//    // Output flags - completely raw
	//    options.c_oflag = 0;
	//
	//    // Local flags - completely raw
	//    options.c_lflag = 0;
	//
	//    // Control characters
	//    options.c_cc[VMIN]  = 0;
	//    options.c_cc[VTIME] = 0;  // doesnt block (we dont need it because of poll that allready block for 1sec)
    
    // Apply settings with TCSAFLUSH to discard data
    tcsetattr(fd, TCSAFLUSH, &options);
    
    // Flush again after configuration
    tcflush(fd, TCIOFLUSH);
    
    // DTR/RTS control - toggle to reset Minitel state
    int status;
    ioctl(fd, TIOCMGET, &status);
    status |= TIOCM_DTR | TIOCM_RTS;
    ioctl(fd, TIOCMSET, &status);
    

    // Wait for Minitel to be ready
    usleep(500000);  // 500ms - give more time
					 //     // STEP 3: Read and discard anything that arrived
    char discard_buf[256];
    int discarded = 0;
    int n;
    while ((n = ::read(fd, discard_buf, sizeof(discard_buf))) > 0) {
        discarded += n;
        std::cout << "Discarding " << n << " startup bytes: ";
        for (int i = 0; i < n; i++) {
            printf("%02X ", (unsigned char)discard_buf[i]);
        }
        std::cout << "\n";
        usleep(10000);  // Small delay between reads
    }
    std::cout << "Discarded " << discarded << " startup bytes total\n";
    
	struct termios current;
	tcgetattr(fd, &current);

	std::cout << "c_cflag: 0x" << std::hex << current.c_cflag << "\n";
	std::cout << "CS7 set: " << ((current.c_cflag & CSIZE) == CS7) << "\n";
	std::cout << "PARENB set: " << ((current.c_cflag & PARENB) != 0) << "\n";
	std::cout << "PARODD set: " << ((current.c_cflag & PARODD) != 0) << "\n\n";
	std::cout << "c_cflag: 0x" << std::hex << current.c_cflag << "\n";
	std::cout << "CS8 set: " << ((current.c_cflag & CSIZE) == CS8) << "\n";
	std::cout << "PARENB: "  << ((current.c_cflag & PARENB) != 0) << "\n";
    return 1;
}

void HardwareSerial::close() {
    if (fd < 0) return;
    
    std::cout << "Close.\n";
    // Restore terminal settings before closing
    // if (tcgetattr(fd, &original_termios_) == 0) {
    //     tcsetattr(fd, TCSANOW, &original_termios_);
    // }
    
    tcflush(fd, TCIOFLUSH);
    ::close(fd);
    fd = -1;  // ✓ Guard against double close
}

