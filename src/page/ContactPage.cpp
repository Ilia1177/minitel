#include "ContactPage.hpp"

// Default constructor
ContactPage::ContactPage(Minitel* minitel): APage(minitel) {return ;}

ContactPage::~ContactPage(void) {}

void ContactPage::display() {
	
	Minitel &m = *_minitel;

	m.send(CLEAR);
	m.update_cursor(5, 10, 0);
	m.set_typo(YELLOW_CHAR, BLUE_BCKG);
	m.send(BLINK_ON);
	m.write_text(" Please provide your \r\n email address:\r\n");
	m.send(BLINK_OFF);
	m.set_typo(WHITE_CHAR, BLACK_BCKG);
}

void ContactPage::register_contact(const std::string& name) {
		std::ofstream file;

		file.open("contacts.txt", std::ios::app);
		if (!file.is_open()) {
			std::cout << "Create contacts.txt:\n";
			std::ofstream create("contacts.txt");
			create.close();
			file.open("story.txt", std::ios::app);
		}
		file << name + "\n";
		file.close();
}

Minitel::State ContactPage::handle_input(const std::string& input) {

		Minitel &m = *_minitel;
		register_contact(input);
		return m.redirect_display(endState);
}
