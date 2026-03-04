#include "ContactPage.hpp"

// Default constructor
ContactPage::ContactPage(Minitel* minitel): APage(minitel) {return ;}

ContactPage::~ContactPage(void) {}

void ContactPage::display() {
	
	Minitel &m = *_minitel;

		m.update_cursor(5, 19, 0);
		m.write_text(" Please provide your email address:\r\n");
}

Minitel::State ContactPage::handle_input(const std::string& input) {

	Minitel &m = *_minitel;
		std::ofstream file;

		file.open("contacts.txt", std::ios::app);
		if (!file.is_open()) {
			std::cout << "Create contacts.txt:\n";
			std::ofstream create("contacts.txt");
			create.close();
			file.open("story.txt", std::ios::app);
		}
		if (!file.is_open()) {
			return m.redirect_display(Minitel::State::MENU);
		}
		file << input + "\n";
		file.close();
		return m.redirect_display(Minitel::State::EMAIL);
}
