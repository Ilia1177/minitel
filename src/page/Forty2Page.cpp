#include "Forty2Page.hpp"

// Default constructor
Forty2Page::Forty2Page(Minitel* minitel): APage(minitel) {return ;}


Forty2Page::~Forty2Page(void) {}

void Forty2Page::display() {
	std::cout << "User get to HAZARDOUS maze\n";
	// std::cout << "\tCurrent: " << get_state(_state) << "\n";
	// std::cout << "\tto     : " << get_state(finaleState) << "\n";
	Minitel &m = *_minitel;
	m.send(CLEAR);
	m.update_cursor(1, 1, 0);
	m.write_text("42’s position is unique in the world of higher education: it is based on the strong value of a sustainable professional integration in the labor market. What makes 42’s training different?");
	m.send(m.set_typo(WHITE_CHAR, BLACK_BCKG));
	m.write_text("Creativity lays on the heart of everyone.\r\n");
	m.write_text("Tools are limitless.........\r\n");
	m.write_text("Create your own, try to be innovative.\r\n");;
	m.write_text("And try to be happy !\r\n");
}

Minitel::State Forty2Page::handle_input(const std::string& input) {
	(void)input;
	return Minitel::State::MENU;
}
