
#include "Server.hpp"

Server::Server() {};
int Server::add_client(const std::string serial_path) {
	HardwareSerial hds;
	Client* new_client;
	pollfd new_pfd;

	if (!hds.openPort(serial_path.c_str())) {
		std::cerr << "fail opening port\n";
		return 1;
	}
	new_client = new Client(hds);
	new_client->index = _clients.size();
	new_pfd.events = POLLIN;
	new_pfd.revents = 0;
	new_pfd.fd = new_client->serial.getFileDescriptor();
	_clients.push_back(new_client);
	_pfds.push_back(new_pfd);
	std::cout << "New client with port ";
	std::cout << serial_path << " added !\n";
	new_client->minitel->newScreen();
	return 0;
}

int Server::handle_input(Client& client) {
	unsigned long key = client.minitel->getKeyCode();
	std::cout << "client key code: " << key << "\n:";
	client.minitel->print("You have printed something\r\n");
	return 0;
}

int Server::listen()
{
	if (_clients.size() < 1) {
		return 1;
	}
	_rbuff.clear();
	std::cout << "start Listening clients\n";
    while (!g_signal) 
	{
        int ret = poll(_pfds.data(), _pfds.size(), POLL_TIMEOUT);
        if (ret < 0) {
            std::cerr << "poll() error: " << strerror(errno) << "\n";
            break;
        } else if (ret == 0) {
            continue;
        }

		for(size_t i = 0; i < _pfds.size(); i++) 
		{
			if (_pfds[i].revents & POLLIN) {
				handle_input(*_clients[i]);
			} else if (_pfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
				std::cerr << "Poll error on serial port\n";
				break;
			}
			_pfds[i].revents = 0;
		}
        // std::cout << "listening: STATUS: " << get_state(_state) + "\n";
    }
	return g_signal;
}
