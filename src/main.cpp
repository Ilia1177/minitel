#include "Minitel.hpp"
#include <signal.h>

bool g_interrupt = false;

void signal_handler(int signum) {
	if (signum == SIGINT)
		g_interrupt = true;
}

int main(int ac, char** av)
{
    Minitel m;

    if (m.configure_serial("/dev/cu.usbserial-0001") < 0) {
		return 1;
	}
	signal(SIGINT, signal_handler);

	m.init(ac, av);

	std::vector<std::string> menu = {
		"1. clear screen", 
		"2. write string", 
		"3. bell",
		"4. mosaique mode",
		"5. ascii mode",
		"6. print ascii table",
		"7. print png",
		"8. cursor at 10 - 0",
		"9. cursor on",
		"10. cursor off",
		"11. exit",
		"12. recovery",
		"13. listen",
		"14. blink on",
		"15. blink off",
		"16. roll mode",
		"17. red chars",
		"18. send ascii art (80 cols)",
	};

	std::ifstream file("ascii/ascii_art.txt");
	std::stringstream buffer;
	buffer << file.rdbuf();
	// m.read();
	std::string line;
	while(!g_interrupt) {
		// m.fsend("SPECIAL SERVER MENU", CLEAR, BLINK_ON, MAGENTA_BCKG);
		// m.fsend("\r\n", BLINK_OFF);
		switch(m.dial_menu(menu)) {
			case 1: m.clear_screen(); break;
			case 2: user_line("write: ", line); m.send(line); break;
			case 3: m.bell(); break;
			case 4: m.mosaique_mode(); break;
			case 5: m.ascii_mode(); break;
			case 6: m.display_ascii_table(); break;
			case 7: m.png_to_mosaique("ascii/img.png"); break;
			case 8: m.cursor_at(10, 1); break;
			case 9: m.cursor_on(); break;
			case 10: m.cursor_off(); break;
			case 11: g_interrupt = true; break;
			case 12: m.recovery(); break;
			case 13: m.listen(); break;
			case 14: m.blink_on(); break; 
			case 15: m.blink_off(); break; 
			case 16: m.roll_mode(); break; 
			case 17: m.test_char(); break; 
			case 18: m.send(buffer.str());
			default:
				break;
		}
	}
    m.send(CLEAR);
    return 0;
}
