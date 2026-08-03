#ifndef SERVER_HPP
# define SERVER_HPP

#include <poll.h>
#include "Minitel1B_Hard.h"
#include "Client.hpp"
# define POLL_TIMEOUT 1000

extern int g_signal;

class Server {
	public:
	Server(void);
	int listen(void);
	int handle_input(Client&);
	int add_client(const std::string serial_path);

	private:
	std::vector<Client*> _clients;
	std::vector<struct pollfd> _pfds;
	std::string _rbuff;	
};

#endif
