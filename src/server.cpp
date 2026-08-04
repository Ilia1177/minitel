
#include "Server.hpp"

Server::Server() {};
int Server::add_client(const std::string serial_path)
{
    HardwareSerial hds;
    Client*        new_client;
    pollfd         new_pfd;

    if (!hds.openPort(serial_path.c_str())) {
        return 1;
	}
	std::cout << "Port " << serial_path << "open\n";
	std::cout << "fd " << hds.getFileDescriptor() << "\n";
    new_client = new Client(hds);
    new_client->index = _clients.size() + 1;
    new_pfd.events = POLLIN;
    new_pfd.revents = 0;
    new_pfd.fd = new_client->serial.getFileDescriptor();

    _clients.push_back(new_client);
    _pfds.push_back(new_pfd);
    std::cout << "New client added:\n";
	std::cout << "\tClient serial fd: " << new_client->serial.getFileDescriptor();
    std::cout << "\tDevice: " << serial_path << "\n";
	init_machine(*new_client);
    return 0;
}

void Server::main_page(Client& client)
{
	Minitel machine = *(client.minitel);
	machine.rect(1, 1, 20, 20);
	machine.noCursor();
	
	machine.moveCursorXY(4, 5);
	machine.attributs(CARACTERE_VERT);
	machine.attributs(INVERSION_FOND);
	machine.println("option 1");
	machine.attributs(FOND_NORMAL);
	machine.moveCursorRight(3);
	machine.println("option 2");
	machine.moveCursorRight(3);
	machine.println("option 3");
}

void Server::init_machine(Client &client)
{
	std::cout << "init screen\n" << std::hex;
	Minitel* machine = client.minitel;
	// std::cout << "reset: " << machine->reset() << "\n";
	std::cout << "new screen: ";
	machine->newScreen();
	std::cout << "cursor\n";
	machine->cursor();
	std::cout << "available before smallMode: " << client.serial.available() << "\n";
	std::cout << "smallMode: " << machine->smallMode();
	// std::cout << "echo off: ";
	// std::cout << machine->echo(false);
	std::cout << "change speed\n";
	machine->changeSpeed(9600);
	std::cout << "print\n";
	machine->print("Screen initialised");
	std::cout << "newscreen\n";
	machine->newScreen();
	std::cout << "main page\n" << std::dec;
	client.currentPage = MAIN_PAGE;
	main_page(client);
}

int Server::main_page_input(Client& client)
{
    unsigned long key = client.minitel->getKeyCode();
	switch (key) {
	case '1':
		printf("1\n");
		break;
	case '2':
		printf("2\n");
		break;
	case '3':
		printf("3\n");
		break;
	case '4':
		printf("Down\n");
		break;
	}
	return 0;
}
int Server::handle_input(Client& client)
{
	switch(client.currentPage) {
		case MAIN_PAGE:
			main_page_input(client);
			break;
		case GAME1:
		default: break;
	}
	//    client.minitel->println("You have printed something 12345678901234556789809012345678901234567890123456789023456789012345678901234567890");
	// if (key < 0x80 && std::isprint(static_cast<unsigned char>(key)))
	// 	client.minitel->printChar(static_cast<char>(key));
	// else
	// 	std::cout << "Key: " << key << std::endl;
	   return 0;
}

int Server::listen()
{
    if (_clients.size() < 1) {
        return 1;
    }
    // Use to listen the server stdin
    // struct pollfd stdinPfd{};
    // stdinPfd.fd = STDIN_FILENO;
    // stdinPfd.events = POLLIN;
    // _pfds.push_back(stdinPfd);
    _rbuff.clear();
    std::cout << "start Listening clients\n";
    while (!g_signal) {
        int ret = poll(_pfds.data(), _pfds.size(), POLL_TIMEOUT);
        if (ret < 0) {
			if (errno == EINTR) // Interrupted by a signal (e.g. Ctrl+C)
            	continue;
			break;
        } else if (ret == 0) {
            continue;
        }
        // stdin is always pfds[0]
        // if (_pfds[0].revents & POLLIN) {
        // 	std::string line;
        // 	std::getline(std::cin, line);
        // 	handleServerCommand(line);
        // }
        for (size_t i = 0; i < _pfds.size(); i++) {
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
    return 0;
}
