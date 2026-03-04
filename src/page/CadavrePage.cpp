#include "CadavrePage.hpp"
// Default constructor

CadavrePage::CadavrePage(Minitel* minitel) : APage(minitel) {}

// Destructor
CadavrePage::~CadavrePage(void) {}

void CadavrePage::display() {
	Minitel &m = *_minitel;

	m.send(CLEAR);
	m.update_cursor(1, 1, 0);
	m.set_typo(WHITE_CHAR, BLACK_BCKG);
	m.write_text("You can allways press 'RETOUR' to go back to menu.\r\n");
	m.send(m.set_typo(BLUE_BCKG, WHITE_CHAR));
	m.write_text("Your turn to say something...");
	m.send(BLINK_ON);
	m.write_text(" -> *PRESS ENTER* to validate your response\r\n");
	m.send(BLINK_OFF);//blink_off();
	m.write_text("Here is what people said...\r\n");
	m.send(m.set_typo(BLUE_BCKG, MAGENTA_CHAR));
	m.send_file("story.txt", 20);
}

Minitel::State CadavrePage::handle_input(const std::string& input) {
	std::cout << "User enter cadavre exquis.\n";
	// std::cout << "\tCurrent: " << get_state(_state) << "\n";
	// std::cout << "\tfinal  : " << get_state(finaleState) << "\n";

	Minitel &m = *_minitel;
		std::ofstream file;

		// Mionitel::State state = finaleState;
		file.open("story.txt", std::ios::in | std::ios::out | std::ios::app);
		if (!file.is_open()) {
			std::cout << "Create story.txt:\n";
			std::ofstream create("story.txt");
			create.close();
			file.open("story.txt", std::ios::in | std::ios::out | std::ios::app);
		}
		if (!file.is_open()) {
			return Minitel::State::MENU;
		}
		size_t CONTENT_WIDTH = 38;
		// size_t CONTENT_WIDTH = COLS_VIDEOTEX - m.marginX * 2;

		size_t col = m.cursorX;

		for (size_t i = 0; i < input.length(); ++i)
		{
			if (col >= CONTENT_WIDTH + 1) {
				file << "\r\n";  // end space + newline
				col = 1;
			}
			file << input[i];
			col++;
		}
		file.flush();
		file.clear();              // clear EOF flags
		file.close();
		m.send(m.set_typo(MAGENTA_CHAR, BLUE_BCKG));
		if (input.length() > 0) {
			m.write_text("\r\n   Thanks you for your participation !!\r\n");
		} 
		// else if (finaleState == State::HAZARDOUS){
		// 	m.cursor_to(1, 1);
		// 	m.write_text("\r\n  Why wont you write something ?");
		// 	m.write_text("\r\n  Communication is a key for social intelligence");
		// 	m.write_text("\r\n  It is also a regular choice... or a mistake ?\r\n");
		// 	m.ascii_noise(100);
		// 	m.send(set_typo(WHITE_CHAR, BLACK_BCKG));
		// 	m.write_text(" What do you think about what you've created ?");
		// 	m.ascii_noise(0);
		// }

		return Minitel::State::HAZARDOUS;
}

