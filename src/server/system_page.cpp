#include "Server.hpp"

void Server::system_page(Client* client)
{
	client->killShell();          // clean up any previous session first
    client->currentPage = SYSTEM;
    client->minitel->clearScreen();
    client->term = new TermScreen(40, 24);
    client->pty  = new Pty();
    client->pty->spawn(40, 24);
	fcntl(client->pty->getFd(), F_SETFL, O_NONBLOCK); // fix blocking read, see below
    rebuildPfds();
}

// Called from handle_client_input when currentPage == SYSTEM
void Server::system_page_input(Client* client)
{
    unsigned long code = client->minitel->getKeyCode(false); // raw, not unicode
    std::string bytes;

    switch (code) {
        case ENVOI:            bytes = "\r"; break;              // Envoi
		case CORRECTION:		bytes = "\x7F"; break;
        case TOUCHE_FLECHE_HAUT:         bytes = "\x1B[A"; break;          // up
        case TOUCHE_FLECHE_BAS:         bytes = "\x1B[B"; break;          // down
        case TOUCHE_FLECHE_DROITE:         bytes = "\x1B[C"; break;          // right
        case TOUCHE_FLECHE_GAUCHE:         bytes = "\x1B[D"; break;          // left
        default:
            if (code < 0x80) bytes += (char)code;
            break;
    }
    if (!bytes.empty()) client->pty->write(bytes.data(), bytes.size());
    return;
}
