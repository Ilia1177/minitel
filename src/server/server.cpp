#include "Server.hpp"

#include <poll.h>

char appendCodepoint(std::string& input, unsigned long code)
{
    if (code == 0) return 0; // no key

    // Filter out non-printable function/navigation codes if you track them separately
    // (e.g. codes still shaped like 0x1B5B.., or bare 0x13.. function keys)

    if (code < 0x80) {
		char c = static_cast<char>(code);
		input += c;
		return c;
    } else {
        // UTF-8 encode the Unicode code point
        if (code < 0x800) {
            input += static_cast<char>(0xC0 | (code >> 6));
            input += static_cast<char>(0x80 | (code & 0x3F));
        } else {
            input += static_cast<char>(0xE0 | (code >> 12));
            input += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
            input += static_cast<char>(0x80 | (code & 0x3F));
        }
    }
	return 0;
}


Server::Server()
{
    struct pollfd stdinPfd{};
    stdinPfd.fd = STDIN_FILENO;
    stdinPfd.events = POLLIN;
    _pfds.push_back(stdinPfd);
};

Server::~Server()
{
    for (size_t i = 0; i < _clients.size(); i++) {
        delete _clients[i];
    }
}

int Server::add_client(const char* device_path)
{
    Client* new_client;
    pollfd  new_pfd;
	int status;

	new_client = nullptr;
	log("Adding new client...", INFO);
	do {
		log("Trying...", INFO);
		if (new_client)
			delete new_client;
		try {
			new_client = new Client(device_path);
    		new_client->index = _clients.size() + 1;
		} catch (std::exception& e) {
			log(e.what(), ERR);
			return -1;
		}
		status = init_machine(new_client);
		if (status < 0) {
			log("fail machine initialisation !", WARN);
		}
	} while(status < 0 && !g_signal);
	if (g_signal)
		return g_signal;
    new_pfd.events = POLLIN;
    new_pfd.revents = 0;
    new_pfd.fd = new_client->serial.getFd();
    _clients.push_back(new_client);
    _pfds.push_back(new_pfd);
	log("New client added !", INFO);
    return 0;
}

int Server::init_machine(Client* client)
{
    Minitel* machine = client->minitel;
    byte     reponse;

	std::stringstream ss;
    ss << "init machine of client " << client->index;
	log(ss.str(), INFO);
	ss.clear();
    machine->newScreen();
    reponse = machine->echo(false);
	if (reponse == 0x44) {
		client->minitel->smallMode();
  		client->minitel->extendedKeyboard();  // Clavier étendu
		main_page(client);
		return 0;
	}
	ss << "Error: answer: 0x" << std::hex << static_cast<unsigned int>(reponse);
	log(ss.str(), ERR);
	return -1;
}

int Server::handle_client_input(Client* client)
{
    if (g_signal) return 0;
    int ret;

	log("Handle client input", INFO);

	ret = 0;
    switch (client->currentPage) {
		case MAIN_PAGE:
			ret = main_page_input(client);
			break;
		case RISO:
			ret = risographie_input(client);
			break;
		case CONNINFO:
			ret = connexion_input(client);
			break;
		default:
			break;
    }
    return ret;
}

std::vector<std::string> parse_command(std::string& cmd)
{
	std::istringstream iss(cmd);
	std::string word;
	std::vector<std::string> args;

	while (iss >> word) {
		args.push_back(word);
	}
	return args;
}
// Client* Server::get_client()

// commands are:
// state -> print state of each clients
// send -> send message to client number
int Server::handle_server_command(std::string& cmd)
{
    std::istringstream iss(cmd);
    std::string        word;
	std::vector<std::string> args;

	args = parse_command(cmd);

	if(args.size() > 0 && args[0] == "send") {
		log("SERVER SEND COMMAND", INFO);
	}
    return 0;
}

void Server::log(std::string str, log_level status)
{
	switch(status) {
		case ERR: 
			std::cout << RED;
			break;
		case WARN:
			std::cout << YELLOW;
			break;
		case INFO:
			std::cout << BLUE;
			break;
	}
	std::cout << str << ENDC << std::endl << std::flush;
	// std::cout << ">" << std::flush;
}

//
// // _pfds[0] contiens stdin pour ecouter les commandes du server
// // _pfds s'étends entre 0 (stdin) et _clients.size (dernier client)
#include "poll.h"
 #include <sys/socket.h>
#include <netinet/in.h>
int Server::listen()
{
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		perror("socket");
		return 1;
	}
	int opt = 1;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(30777);

	if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return 1;
	}
	if (::listen(listen_fd, 16) < 0) {
		perror("listen");
		return 1;
	}

	// Make it non-blocking so a stalled accept() never freezes poll()
	int flags = fcntl(listen_fd, F_GETFL, 0);
	fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK);

	pollfd listen_pfd{};
	listen_pfd.fd = listen_fd;
	listen_pfd.events = POLLIN;
	_pfds.push_back(listen_pfd); 

    // if (_clients.size() < 1)
        // return 1;

    log("start Listening clients", INFO);
    while (!g_signal) {
        int ret = poll(_pfds.data(), _pfds.size(), POLL_TIMEOUT);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            break;
        } else if (ret == 0) {
            continue;
        }

	for (size_t i = 0; i < _pfds.size(); i++) {
	    if (i == 0 && _pfds[0].revents & POLLIN) {
		log("Listen from STDIN",INFO);
		std::string line;
		std::getline(std::cin, line);
		handle_server_command(line);
	    } else if (_pfds[i].fd == listen_fd && _pfds[i].revents & POLLIN) {
		log("ADD PFD FROM PORT 30777", WARN);
		sockaddr_in client_addr{};
		socklen_t len = sizeof(client_addr);
		int client_fd = accept(listen_fd, (sockaddr*)&client_addr, &len);
		if (client_fd >= 0) {
			fcntl(client_fd, F_SETFL, O_NONBLOCK); // non-blocking client too
			pollfd new_pfd{};
			new_pfd.fd = client_fd;
			new_pfd.events = POLLIN;
			_pfds.push_back(new_pfd);

		}
	    } else if (_pfds[i].revents & POLLIN) {
		handle_client_input(_clients[i - 2]);
	    } else if (_pfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
		log("Poll error on serial port", ERR);
		break;
	    }
	    _pfds[i].revents = 0;
	}
    }
    return 0;
}

int Server::handle_client_command(Client* client, std::string& command)
{
	std::vector<std::string> args = parse_command(command);
	if(args.size() <= 0) {
		command.clear();

	} else if(args[0] == "menu") {
		main_page(client);
	} else if (args[0] == "riso") {
		risographie_page(client);
	} else if (args[0] == "connexion") {
		connexion_page(client);
	} else {
		client->minitel->moveCursorUp(1);
		client->minitel->moveCursorLeft(40);
		client->minitel->printChar('\r');
		client->minitel->print("COMMAND NOT HANDLE");
	}
	command.clear();
	machine_print_infos(client);
	return 0;
}
