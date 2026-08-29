
#include "Server.hpp"

#include <poll.h>
#include <random>

void printinfos(Client* client)
{
    Minitel* machine = client->minitel;

    machine->moveCursorXY(1, 24);
    machine->attributs(CARACTERE_BLANC);
    machine->attributs(INVERSION_FOND);
    machine->print("ANNULATION:");
    machine->attributs(FOND_NORMAL);
    machine->print("menu");
    machine->moveCursorRight(4);
}

void input_box(Client* client)
{
	Minitel* machine;

	static std::string input;

	machine = client->minitel;
	machine->moveCursorXY(1, 24);
	machine->attributs(CARACTERE_VERT);
	machine->attributs(INVERSION_FOND);
	machine->cancel();
	machine->attributs(FOND_NORMAL);
	machine->scrollMode();
	machine->print(">");
	machine->cursor();
	machine->echo(true);
}

Server::Server()
{
    // Use to listen the server stdin
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
			log("fail init machine !", WARN);
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

void Server::main_page(Client* client)
{
    Minitel machine = *(client->minitel);

    client->currentPage = MAIN_PAGE;
    machine.newScreen();
    machine.println00("Laboratoire de création informatique");
    machine.attributs(CARACTERE_BLANC);
    machine.attributs(INVERSION_FOND);
    machine.println();
    machine.println();
    machine.print("Bienvenue au laboratoire d'impression numerique. Press the number of your "
                  "choice to make your choice.");
    machine.cancel();
    machine.attributs(FOND_NORMAL);
    machine.rect(5, 7, 35, 18);
    // machine.noCursor();

    machine.moveCursorXY(8, 10);
    // machine.attributs(CARACTERE_VERT);
    // machine.attributs(INVERSION_FOND);
    machine.println("1. MAIN PAGE");
    machine.moveCursorRight(7);
    machine.println("2. Game one");
    machine.moveCursorRight(7);
    machine.println("3. Talk to gutenberg");
    machine.moveCursorRight(7);
    machine.println("4. Learn about light");
    machine.moveCursorRight(7);
    machine.println("5. [ ... ]");
    machine.moveCursorRight(7);
    machine.println("6. (DEV) EXIT SERVER");

    machine.moveCursorXY(1, 20);

    machine.attributs(CARACTERE_BLANC);
    machine.attributs(INVERSION_FOND);
    machine.print("Explore l'impression numerique");
    machine.cancel();
    machine.attributs(FOND_NORMAL);
    printinfos(client);
}

void Server::game_one(Client* client)
{
    Minitel* machine = client->minitel;
    // Seed a Mersenne Twister random number engine with a real-time clock
    std::random_device rd;
    std::mt19937       gen(rd());

    client->currentPage = GAME1;

    // Specify a uniform distribution between 0 and 100 (inclusive)
    std::uniform_int_distribution<> disX(1, 40);
    std::uniform_int_distribution<> disY(1, 25);

    for (int i = 0; i < 20; i++) {
    }
    int ranX = disX(gen);
    int ranY = disY(gen);

    machine->newScreen();
    machine->moveCursorXY(ranX, ranY);
    machine->printChar('a');
	input_box(client);
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
		client->currentPage = MAIN_PAGE;
		main_page(client);
		return 0;
	}
	ss << "Error: answer: 0x" << std::hex << static_cast<unsigned int>(reponse);
	log(ss.str(), ERR);
	return -1;
}

int Server::main_page_input(Client* client)
{
    unsigned long key = client->minitel->getKeyCode();
    switch (key) {
    case '1':
        main_page(client);
        break;
    case '2':
        game_one(client);
        break;
    case '3':
        printf("3\n");
        break;
    case '4':
        printf("4\n");
        break;
    case '5':
        printf("4\n");
        break;
    case '6':
    case ANNULATION:
        return 1;
    default:
        printf("Wrong button pressed.\n");
    }
    return 0;
}

int Server::game_one_input(Client* client)
{
	static std::string input;
    unsigned long key = client->minitel->getKeyCode();
    switch (key) {
    case ANNULATION:
        main_page(client);
        break;
	case '\n':
		std::cout << "Client " << client->index << " :" << input << std::endl;
		input = "";
		break;
    default:
		input += std::string(1, static_cast<char>(key));
        break;
    }
    return 0;
}

int Server::handle_client_input(Client* client)
{
    int ret = 0;
    switch (client->currentPage) {
    case MAIN_PAGE:
        ret = main_page_input(client);
        break;
    case GAME1:
        ret = game_one_input(client);
        break;
    default:
        break;
    }
    return ret;
}

// Client* Server::get_client()

// commands are:
// state -> print state of each clients
// send -> send message to client number
int Server::handle_server_command(std::string cmd)
{
    std::istringstream iss(cmd);
    std::string        word;

    iss >> word;
	if (word == "send") {
		iss >> word;
		int index;
		try {
    		index = std::stoi(word);
			if (index > (int)_clients.size() || index <= 0)
				throw std::out_of_range("bad index: " + word);
		} catch (const std::exception& e) {
    		log(e.what(), ERR);
			return 1;
		}
		std::string message;
		std::getline(iss >> std::ws, message);
		_clients[index]->minitel->moveCursorXY(1, 24);
		_clients[index]->minitel->print("SERVER MSG: ");
		_clients[index]->minitel->println(message);
		std::stringstream ss;
		ss << "Message sent to client " << index;
		log(ss.str(), INFO);
	} else if (word == "state") {
		for(size_t i = 0; i < _clients.size(); i++) {
			std::stringstream ss;
			ss << "Client " << _clients[i]->index << ": ";
			ss << _clients[i]->current_page_str();
			log(ss.str(), INFO);
		}
	} else {
		std::stringstream ss;
		ss << "Command '" << word << "' not handle";
		log(ss.str(), WARN);
	}
    return 0;
}

void Server::log(std::string str, error_status status)
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

int Server::listen()
{
    if (_clients.size() < 1)
        return 1;

    log("start Listening clients", INFO);
    while (!g_signal) {
        int ret = poll(_pfds.data(), _pfds.size(), POLL_TIMEOUT);
        if (ret < 0) {
            if (errno == EINTR) // Interrupted by a signal (e.g. Ctrl+C)
                continue;
            break;
        } else if (ret == 0) {
            continue;
        }
        // _pfds[0] contiens stdin pour ecouter les commandes du server
		// _pfds s'étends entre 0 (stdin) et _clients.size()
        for (size_t i = 0; i < _pfds.size(); i++) {
            if (i == 0 && _pfds[0].revents & POLLIN) {
                std::string line;
                std::getline(std::cin, line);
                handle_server_command(line);
            } else if (_pfds[i].revents & POLLIN) {
                if (handle_client_input(_clients[i - 1]))
                    return 0;
            } else if (_pfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                log("Poll error on serial port", ERR);
                break;
            }
            _pfds[i].revents = 0;
        }
    }
    return 0;
}
