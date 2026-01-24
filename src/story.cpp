#include "Minitel.hpp"
#include <unistd.h>
#include <fstream>


void Minitel::cadavre_exquis(const std::string& input)
{
	std::cout << "Cadavre exquis with input: " << input << "\n";
	if (!input.empty()) {
		if (storyBook_.is_open()) {
			storyBook_ << input;
			storyBook_ << "\n";
			std::cout << "Write into story book.\n";
		}
		clear_screen();
		story_ += "\r\n";
		story_.append(input);
		state_ = State::MENU;
		send("Thanks you for your participation.\r\n");
		send("You are gonna be redirected to main menu in 5 sec.");
		sleep(5);
		return;
	}
	clear_screen();
	send(WHITE_BCKG);
	send(BLACK_CHAR);
	// write(serial_port_, WHITE_BCKG, 2);
	// write(serial_port_, BLACK_CHAR, 2);
	send(" Continue the story !            \r\n");
	send(WHITE_BCKG);
	send(BLACK_CHAR);
	// write(serial_port_, WHITE_BCKG, 2);
	// write(serial_port_, BLACK_CHAR, 2);
	send(" Press RETOUR to go back to menu.\r\n");
	send(WHITE_BCKG);
	send(BLACK_CHAR);
	// write(serial_port_, WHITE_BCKG, 2);
	// write(serial_port_, BLACK_CHAR, 2);
	send(" This is the las story line:     \r\n");
	send(BLACK_BCKG);
	send(WHITE_CHAR);
	// write(serial_port_, BLACK_BCKG, 2);
	// write(serial_port_, WHITE_CHAR, 2);

	std::string storyEnd;
    if (story_.length() <= 40) {
        storyEnd = story_;
    } else {
		storyEnd = story_.substr(story_.length() - 40);
	}
	send(storyEnd + "\r\n");
	blink_on();
	send("Your turn: \r\n");
	blink_off();
}

void Minitel::forty_two(const std::string& input) {
	clear_screen();
	if (!input.empty()) {
		state_ = State::MENU;
		return;
	}
	fsend("Creativity lays on the heart of everyone.\
			The tools are limitless, but speechless.\
			Create your own, and search for your innovation.", WHITE_BCKG, BLACK_CHAR);
	
}
