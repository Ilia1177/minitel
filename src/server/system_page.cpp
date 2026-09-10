#include "Server.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>
extern volatile sig_atomic_t g_signal;

static void sanitizeTeleinfoBuf(const char* data, size_t len, std::string &out){
    out.reserve(len);
    for(size_t i=0;i<len;i++){
        unsigned char c=(unsigned char)data[i];
        if(c==0x1B){ out.push_back(c); continue; }
        if(c==0x07 || c==0x00) continue; // BEL/NUL -> drop (white block)
        if(c==0x7F){ out.append("\x08 \x08",3); continue; } // DEL -> erase
        if(c < 0x20){
            if(c=='\r' || c=='\n' || c=='\b' || c=='\t') out.push_back(c);
            continue;
        }
        if(c < 0x7F){ out.push_back(c); continue; }
        if(c>=0x80) continue; // drop high bytes
    }
}
static void drainToSerialFd(int sfd, const char* data, size_t len){
    extern volatile sig_atomic_t g_signal;
    size_t off=0;
    while(off<len){
        if(g_signal) break;
        ssize_t w = ::write(sfd, data+off, len-off);
        if(w>0){ off+=(size_t)w; continue; }
        if(w<0 && (errno==EAGAIN || errno==EWOULDBLOCK)){
            struct pollfd wf{ sfd, POLLOUT, 0 };
            poll(&wf,1,200);
            continue;
        }
        break;
    }
}
void Server::system_page_output(Client* client, const char* data, size_t len){
    std::string filtered;
    sanitizeTeleinfoBuf(data, len, filtered);
    if(filtered.empty()) return;
    drainToSerialFd(client->serial.getFd(), filtered.data(), filtered.size());
}
void Server::system_page(Client* client)
{
	client->killShell();          // clean up any previous session first
    // Fix double echo: set aiguillage while still in Videotex where PRO3 is valid
    // Do this BEFORE standardTeleinformatique() — after switch Teleinfo ignores PRO3 from prise
    client->minitel->aiguillage(false, CODE_EMISSION_CLAVIER, CODE_RECEPTION_ECRAN);
    usleep(100000);
    client->minitel->aiguillage(true, CODE_EMISSION_CLAVIER, CODE_RECEPTION_PRISE);
    usleep(100000);
    client->minitel->aiguillage(true, CODE_EMISSION_PRISE, CODE_RECEPTION_ECRAN);
    usleep(100000);
    // Switch Minitel to Teleinfo (ASCII / 80 cols) — transparent vt100 passthrough
    // Keep 7E1 for Teleinfo — Minitel expects 7E1 even in Teleinfo (parity-checked)
    client->minitel->standardTeleinformatique();
    usleep(300000);
    client->minitel->scrollMode();
    usleep(150000);
    // Ensure serial stays 7E1/CLOCAL after switch (Teleinfo reset may touch termios)
    {
        int fd = client->serial.getFd();
        termios tty{};
        if (tcgetattr(fd, &tty) == 0) {
            speed_t curIn = cfgetispeed(&tty);
            speed_t curOut = cfgetospeed(&tty);
            cfmakeraw(&tty);
            tty.c_cflag &= ~(CSIZE | CSTOPB | PARENB);
            tty.c_cflag |= CS7 | PARENB | CLOCAL | CREAD;
            tty.c_cc[VMIN] = 0; tty.c_cc[VTIME] = 0;
            cfsetispeed(&tty, curIn);
            cfsetospeed(&tty, curOut);
            tcsetattr(fd, TCSANOW, &tty);
            tcflush(fd, TCIOFLUSH);
        }
    }
    usleep(200000);

    client->currentPage = SYSTEM;
    // In Teleinfo we don't use Videotex clearScreen — just reset via ANSI
    // Send CSI 2J + CSI H to clear and home (works in Teleinfo/vt100)
    // Ensure cursor visible (CSI ?25h) and scroll region reset CSI r
    {
        const char cls[] = "\x1B[r\x1B[?25h\x1B[2J\x1B[H";
        ::write(client->serial.getFd(), cls, sizeof(cls)-1);
    }
    // Spawn 80x24 pty — Teleinfo is 80 cols, not 40
    client->pty  = new Pty();
    // Keep TermScreen allocated for compat but not used in Teleinfo path
    // client->term = new TermScreen(80, 24);
    client->term = nullptr;
    client->pty->spawn(80, 24);
    fcntl(client->pty->getFd(), F_SETFL, O_NONBLOCK); // non-blocking read in poll loop
    rebuildPfds();
    log("SYSTEM: entered Teleinfo 80x24", INFO);
}

// Called from handle_client_input when currentPage == SYSTEM
// In Teleinfo the Minitel sends raw ASCII/ANSI directly — we forward via serial
void Server::system_page_input(Client* client)
{
    extern volatile sig_atomic_t g_signal;
    if (g_signal) return;
    // In Teleinfo we bypass Minitel::getKeyCode Videotex decoder when possible
    // but keep mapping for function keys that still emit Videotex sequences
    unsigned long code = client->minitel->getKeyCode(false); // raw, not unicode
    if (g_signal) return;
    if (code == 0) return;
    std::string bytes;

    switch (code) {
        case ENVOI:            bytes = "\r"; break;
        case RETOUR:           bytes = "\x1B[A"; break; // Shift-Envoi variants map via getKeyCode
        case CORRECTION:       bytes = "\x7F"; break; // DEL / backspace
        case ANNULATION:       // exit shell -> back to Teletel menu
            client->killShell(); // restores 7E1 + Teletel + usleep
            client->currentPage = MAIN_PAGE;
            main_page(client);
            rebuildPfds();
            return;
        case CONNEXION_FIN: // same as ANNULATION for shell
            client->killShell();
            client->currentPage = MAIN_PAGE;
            main_page(client);
            rebuildPfds();
            return;
        case SOMMAIRE:             bytes = "\x1B[H"; break; // Home
        case GUIDE:                bytes = "\x1BOP"; break;
        case SUITE:                bytes = "\x1B[6~"; break;
        case TOUCHE_FLECHE_HAUT:   bytes = "\x1B[A"; break;
        case TOUCHE_FLECHE_BAS:    bytes = "\x1B[B"; break;
        case TOUCHE_FLECHE_DROITE: bytes = "\x1B[C"; break;
        case TOUCHE_FLECHE_GAUCHE: bytes = "\x1B[D"; break;
        default:
            if (code < 0x80) {
                bytes += (char)code;
            } else if (code < 0x800) {
                // UTF-8 passthrough for vt100 — send as UTF-8 bytes if term supports
                // For now drop, could map via Minitel::getString
            }
            break;
    }
    if (!bytes.empty() && client->pty) client->pty->write(bytes.data(), bytes.size());
    return;
}
