#include "Minitel.hpp"
#include <unistd.h>
#include <fstream>

void Minitel::hazardous_collective(const std::string& input) {
	if (!input.empty()) {
		_state = State::MENU;
		return ;
	}
	else
	{
		send(CLEAR);
		_cursorY = 1;
		_cursorX = 1;
		write_text("Welcome to hazardous\r\nPRESS ANY KEY TO CONTINUE\r\n");
	}
}

void Minitel::cadavre_exquis(const std::string& input)
{	
	std::cout << "Cadavre exquis with input: " << input << "\n";
	if (!input.empty()) {
		_storyBook.open("story.txt", std::ios::in | std::ios::out | std::ios::app);
		if (!_storyBook.is_open()) {
			std::cout << "Create story.txt:\n";
			std::ofstream create("story.txt");
			create.close();
			_storyBook.open("story.txt", std::ios::in | std::ios::out | std::ios::app);
		}
		if (!_storyBook.is_open())
			return;
		constexpr size_t CONTENT_WIDTH = 38;

		size_t col = 0;

		for (size_t i = 0; i < input.length(); ++i)
		{
			_storyBook << input[i];
			col++;

			if (col == CONTENT_WIDTH)
			{
				_storyBook << "\r\n";  // end space + newline
				col = 0;
			}
		}
		_storyBook.flush();
		std::cout << "Write into story book.\n";
		// _story += "\r\n";
		// _story.append(input);
		_state = State::MENU;
		_storyBook.clear();              // clear EOF flags
		_storyBook.close();
		write_text("Thanks you for your participation !!\r\n");

		sleep(2);
		return;
	}
	send(CLEAR);
	_cursorY = 1;
	_cursorX = 1;
	set_typo(WHITE_CHAR, BLACK_BCKG);
	write_text("Continue the story !            \r\n");
	write_text("Press RETOUR to go back to menu.\r\n");
	send(set_typo(BLUE_BCKG, WHITE_CHAR));
	send(BLINK_ON);
	write_text("Your turn: *PRESS ENTER* to validate\r\n");
	send(BLINK_OFF);//blink_off();
		
	write_text("This is the las story line:     \r\n");

		send(set_typo(BLUE_BCKG, MAGENTA_CHAR));
		send_file("story.txt");
		// std::stringstream buffer;
		// std::cout << "clear story\n";
		// _storyBook.clear();
		// std::cout << "seek start\n";
		// _storyBook.seekg(0, std::ios::beg);  // move read pointer to beginning
		// buffer << _storyBook.rdbuf();

		// std::string storyEnd = buffer.str();
		// if (storyEnd.length() > 40*10) {
		// 	storyEnd = storyEnd.substr(storyEnd.length() - 40*10);
		// }
		// write_text(storyEnd + "\r\n", 0);
		std::cout << "finish\n";

	// _storyBook.close();
}

void Minitel::forty_two(const std::string& input) {
	send(CLEAR);
	_cursorX = 1;
	_cursorY = 1;
	if (!input.empty()) {
		_state = State::MENU;
		return;
	}
	send(set_typo(WHITE_CHAR, BLACK_BCKG));
	write_text("Creativity lays on the heart of everyone.\r\n");
	write_text("Tools are limitless, but speechless.\r\n");
	write_text("Create your own, and search for your innovation.\r\n");;
	write_text("PRESS ANY KEY TO GO BACK TO MENU\r\n");
}
