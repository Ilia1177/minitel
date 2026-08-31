#include "server.hpp"

void machine_print_infos(Client* client)
{
    Minitel* machine = client->minitel;

    machine->moveCursorXY(1, 24);
    machine->attributs(CARACTERE_BLANC);
    machine->attributs(INVERSION_FOND);
    machine->print("ANNULATION:");
    machine->print("menu");
    machine->attributs(FOND_NORMAL);
	machine->moveCursorRight(5);
    machine->attributs(CARACTERE_BLANC);
	machine->attributs(INVERSION_FOND);
    machine->print(">");
	machine->cancel();
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

int print_file(Client* client, const std::string &path) 
{
	std::ifstream fichier;
	std::string ligne;
	Minitel* machine;
	byte caractere;

	machine = client->minitel;
	fichier.open(path);
	if (!fichier.is_open()) {
		return -1;
	}

	while(std::getline(fichier, ligne)) {
		machine->println(ligne);
	}
	fichier.close();
	return 0;
}
