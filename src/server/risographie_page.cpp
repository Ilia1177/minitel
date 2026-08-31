#include "Server.hpp"
#include <random>

void Server::risographie_page(Client *client)
{
	client->currentPage = RISO;

	if (print_file(client, "pedago/riso.txt") < 0) {
		log("error printing file", ERR);
		main_page(client);
	}
	machine_print_infos(client);
}
// void Server::risographie_page(Client* client)
// {
//     Minitel*           machine;
//     std::random_device rd;
//     std::mt19937       gen(rd());
//
//     machine = client->minitel;
//     client->currentPage = GAME1;
//
//     std::uniform_int_distribution<> disX(1, 40);
//     std::uniform_int_distribution<> disY(1, 25);
//     std::uniform_int_distribution<> dischar(33, 126);
//
//     machine->newScreen();
//     for (int i = 0; i < 20; i++) {
//         int  ranX = disX(gen);
//         int  ranY = disY(gen);
//         char caractere = dischar(gen);
//         machine->moveCursorXY(ranX, ranY);
//         machine->printChar(caractere);
//     }
//     machine_print_infos(client);
// }

int Server::risographie_input(Client* client)
{
    static std::string input;
    unsigned long      key;
    char               c;
    Minitel*           machine;

	c = 0;
    machine = client->minitel;
    key = machine->getKeyCode();
    switch (key) {
    case ANNULATION:
        main_page(client);
        break;
    case '\r':
	case ENVOI:
        handle_client_command(client, input);
        break;
    default:
        if (input.length() < 15)
        	c = appendCodepoint(input, key);
		machine->printChar(c);
    }
    return 0;
}
