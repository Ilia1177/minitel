#include "Server.hpp"
#include <fstream>

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

std::string getline_number(const std::string &path, int nb)
{
	std::ifstream fichier;
	std::string ligne;

	if (nb < 0)
		return "";
	fichier.open(path);
	if (!fichier.is_open()) {
		return "";
	}
	int i = 0;
	while(std::getline(fichier, ligne) && i < nb)
		i++;
	fichier.close();
	return ligne;
}

int print_file(Client* client, const std::string &path) 
{
	std::ifstream fichier;
	std::string ligne;
	Minitel* machine;
	// byte caractere;

	machine = client->minitel;
	fichier.open(path);
	if (!fichier.is_open()) {
		return -1;
	}
	while(std::getline(fichier, ligne)) {
		machine->println(ligne);
		usleep(1100);
	}
	fichier.close();
	return 0;
}

#include "Pty.hpp"
#include "TermScreen.hpp"
void renderToMinitel(Minitel* m, TermScreen& screen)
{
    auto diff = screen.render();
    for (auto& [x, y, ch] : diff.cells) {
        m->moveCursorXY(x + 1, y + 1); // Minitel coords are 1-based
        m->printChar(ch);
    }
    m->moveCursorXY(diff.cx + 1, diff.cy + 1);
}
