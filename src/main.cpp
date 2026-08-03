// #include "Minitel.hpp"
#include <signal.h>
#include "Server.hpp"

int g_signal = 0;

void signal_handler(int signum) {
	g_signal = signum;
}

int main(int ac, char *av[]) 
{
    // HardwareSerial serialPort;
	Server server;
	if (ac < 2) {
		std::cerr << "Must have an argument: path of device\n";
		return 1;
	}
	signal(SIGINT, signal_handler);

	for(int i = 0; i < ac; i++) {
		server.add_client(av[i + 1]);
	}
	server.listen();

    // minitel.newScreen();
    // minitel.print("Bonjour Minitel");
    //
    // while (true) {
    //     unsigned long key = minitel.getKeyCode();
    //     if (key) { /* handle it */ }
    // }
}
