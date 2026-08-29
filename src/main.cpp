// #include "Minitel.hpp"
#include "Server.hpp"
#include <signal.h>

int g_signal = 0;

void signal_handler(int signum) { g_signal = signum; }

int main(int ac, char* av[])
{
    Server server;
    int    status;

    signal(SIGINT, signal_handler);
    if (ac < 2) {
        std::cerr << "Must have an argument: path of device\n";
        return 1;
    }
    for (int i = 1; i < ac; i++) {
        server.add_client(av[i]);
    }
	try {
    	status = server.listen();
	} catch (std::exception &e) {
		server.log(e.what(), ERR);
	}
    std::cout << "Server stop with status: " << status << std::endl;

    return status;
}
