#include "MazePage.hpp"

// Default constructor
MazePage::MazePage(Minitel* minitel): APage(minitel), _level(1) {
	return ;}

// Destructor
MazePage::~MazePage(void) {}

Minitel::State MazePage::handle_input(const std::string& input)
{
	std::cout << "HAZARDOUS maze > handle user input: " << input << "\n";
	std::cout << "\tlevel: " << _level << "\n";

	Minitel &m = *_minitel;
	// if (input.empty()) {
	// 	_level = 1;
	// 	return Minitel::State::HAZARDOUS;
	// }
	if (_level == 2) {
		std::cout << "\tredirect input to " << m.get_state(Minitel::State::STORY) << "\n";
		m.redirect_input(Minitel::State::STORY, input);
	} else if (_level >= 8) {
		_level = 1;
		std::cout << "\tend maze finish " << "\n";
		return m.redirect_display(Minitel::State::MENU, false);
	} else {
		std::cout << "\tlevel++: " << _level << "\n";
	}
	_level++;
	return m.redirect_display(Minitel::State::HAZARDOUS, false);
}

void MazePage::display()
{
	// _state = State::HAZARDOUS;
	static std::string phrase("");

	Minitel &m = *_minitel;
	// std::string word = input;
	std::cout << "User enter HAZARDOUS maze.\n";
	// std::cout << "\tinput  : " << input << "\n";
	// std::cout << "\tlevel  : " << _level << "\n";
	// std::cout << "\tCurrent: " << get_state(_state) << "\n";
	// std::cout << "\tto     : " << get_state(endState) << "\n";
	// if (input == "exit" || input == "EXIT") {
	// 	_level = 1;
	// 	phrase = "";
	// 	return m.redirect_to(State::MENU);
	// }
	m.send(CLEAR);
	m.send(m.set_typo(WHITE_CHAR, BLACK_BCKG));
	m.ascii_noise(10 * _level);
	if (_level == 1) {
		m.ascii_noise(0);
		m.write_text("... THERE iS No HAZzARD..? ");
		m.ascii_noise(0);
		m.write_text("Alea & complexe systems gives what they want");
		m.ascii_noise(0);
		m.write_text("If your willing to help, please provide an input..."); 	
		m.ascii_noise(0);
	} else if (_level == 2) {
		// if (input.length() > 24) {
		// 	m.update_cursor(1, ROWS_VIDEOTEX, 0);
		// 	m.write_text("We'll just take one letter, because its too longs");
		// 	m.write_text("       Dont make it to difficult...\r\n");
		// 	m.write_text("if you want to leaves...\r\nthen you can press ANNULATION");
		// 	m.write_text(" else you type enter...\r\nthen you can press ANNULATION");
		// } else {
			m.send(m.set_typo(CYAN_CHAR, WHITE_BCKG));
			// m.write_text(" " + input);
			m.send(m.set_typo(RED_CHAR, CYAN_BCKG));
			m.write_text(" is a really nice choice !");
			// m.write_text(" this is " + std::to_string(input.length()) + " characters.");
			m.send(m.set_typo(WHITE_BCKG, BLACK_CHAR));
			m.write_text("\r\n    Now you ll be requested to particapte...");
			m.write_text("\r\nplease.. DO YOUR BEST !");
			m.redirect_display(Minitel::State::STORY);
		// }
	} else if (_level == 3) {
		m.ascii_noise(20);
		m.send(m.set_typo(MAGENTA_CHAR, BLUE_BCKG));
		m.write_text(" Please tell us your next word you have in mind...");
		m.update_cursor(COLS_VIDEOTEX / 2 - 8, ROWS_VIDEOTEX / 2, 0);
		m.send(m.set_typo(WHITE_CHAR, BLACK_BCKG));
	} else if (_level == 4) {
		m.write_text("Would you like to receive\r\nthe final collaborative artwork\r\nby email ? (yes/no)\r\n");
	} else if (_level == 5) {
		// if (input == "yes") { 
			// _state = add_contact(State::HAZARDOUS); 
			// word.clear();
		// }
		// else { m.write_text("What a shame... Why not ?"); }
	} else if (_level == 6) {
		m.write_text("Congratulation ! you reach _level " + std::to_string(_level) + "\r\n");
		m.write_text("what do you think of?\r\n");
	} else if (_level == 7) {
		m.send(m.set_typo(WHITE_BCKG, BLACK_CHAR));
		m.write_text("\r\nYou have told us: \r\n");
		// m.write_text(phrase + " " + word);
		m.send(m.set_typo(WHITE_CHAR, BLACK_BCKG));
		m.write_text("\r\npress any key to continue...\r\n");
	} else {
		phrase = "";
		m.update_cursor(COLS_VIDEOTEX / 2 - 8, ROWS_VIDEOTEX / 2 - 1, 0);
		m.write_text("Hope to see you soon !");
		// _state = redirect_to(endState);
		// return Minitel::State::MENU;
	}
	// if (!word.empty())
		// phrase += " " + word;
	// _level++;

	// return Minitel::State::HAZARDOUS;
}
