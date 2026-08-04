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
	int status;
	if (ac < 2) {
		std::cerr << "Must have an argument: path of device\n";
		return 1;
	}
	signal(SIGINT, signal_handler);
	std::cout << "adding new client...\n";
	for(int i = 1; i < ac; i++) {
		server.add_client(av[i]);
		std::cout << "Add new client: " << av[i] << "\n";
	}
	status = server.listen();
	std::cout << "Server stop with status: " << status << std::endl;

	return status;
    // minitel.newScreen();
    // minitel.print("Bonjour Minitel");
    //
    // while (true) {
    //     unsigned long key = minitel.getKeyCode();
    //     if (key) { /* handle it */ }
    // }
}
