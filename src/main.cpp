#include "Minitel.hpp"
#include <signal.h>

bool g_interrupt = false;

void signal_handler(int signum) {
	if (signum == SIGINT)
		g_interrupt = true;
}

int main(int ac, char** av)
{
    Minitel m;

	signal(SIGINT, signal_handler);

	int status = m.init(ac, av);
	if (status != 0)
		return status;

	std::vector<std::string> menu = {
		"exit", 
		"clear", 
		"start 1200bds",
		"start 9600bds",
		"print file.txt",
	};

	// m.read();
	std::string line;
	while(!g_interrupt) {
		switch(m.dial_menu(menu)) {
			case 1: g_interrupt = true; break;
			case 2: m.send(CLEAR); break;
			case 3: m.start(); break;
			case 4: m.send(SPEED_9600); m.start(); break;
			case 5: m.send_file("ascii/ascii_art.txt");
			default:
				break;
		}
	}
    m.send(CLEAR);
    return 0;
}
