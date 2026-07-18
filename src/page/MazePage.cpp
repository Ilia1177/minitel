#include "MazePage.hpp"
#include "CadavrePage.hpp"
#include "ContactPage.hpp"
// Default constructor
MazePage::MazePage(Minitel* minitel): APage(minitel), _level(1) {
	return ;}

// Destructor
MazePage::~MazePage(void) {}

void MazePage::reset() {
	_level = 1;
	phrase = "";
}

Minitel::State MazePage::handle_input(const std::string& input)
{
	Minitel &m = *_minitel;
	Minitel::State nextState = Minitel::State::HAZARDOUS;
	
	std::cout << "HAZARDOUS maze > handle user input: " << input << "\n";
	std::cout << "\tend state: " << m.get_state(endState) << "\n";
	std::cout << "\tlevel: " << std::dec << _level << "\n";
	std::cout << "\tphrase: " << std::dec << phrase << "\n";

	switch (_level) {
		case 1:
			phrase += input; 
			break;
		case 2:
			m.cadavre->endState = Minitel::State::HAZARDOUS;
			_level++;
			return m.redirect_input(Minitel::State::STORY, input);
		case 3:
			if (input == "left") {
				
			} else if (input == "right") {

			}
			phrase += " " + input; 
			break;
		case 4:
			if (input == "yes") {
				m.contact->endState = Minitel::State::HAZARDOUS;
				nextState = Minitel::State::EMAIL;
			}
			else
			{
				m.write_text("What a shame... why ?\n\r");
				m.ascii_noise(20);
				break;
			}
			break;
		case 5:
			phrase += " " + input; 
			break;
		case 6:
			phrase += " " + input; 
			break;
		case 7:
			phrase += " " + input; 
			break;
		case 8:
			phrase.clear();
			_level = 0;
			nextState = endState;
			break;
	}
	_level++;
	return m.redirect_display(nextState, false);
}

void MazePage::display()
{
	static std::string phrase("");

	Minitel &m = *_minitel;
	// std::string word = input;
	std::cout << "User enter HAZARDOUS maze:\n";
	std::cout << "\tat level " << _level << "\n";
	m.send(CLEAR);
	m.send(m.set_typo(WHITE_CHAR, BLACK_BCKG));
	m.ascii_noise(_level);
	switch (_level) {
		case 1:
			m.ascii_noise(0);
			m.write_text("... THERE iS No HAZzARD.. (?) ");
			m.ascii_noise(0);
			m.write_text("You are now in complexe system");
			m.ascii_noise(0);
			m.write_text("You can allways get back to the main page by pressing 'ANNULATION'");
			m.ascii_noise(0);
			m.write_text("To continue, please provide any input");
			m.ascii_noise(0);
			break;
		case 2:
			m.send(m.set_typo(CYAN_CHAR, WHITE_BCKG));
			m.send(m.set_typo(RED_CHAR, CYAN_BCKG));
			m.write_text(" you might be questionning yourself...");
			m.send(m.set_typo(WHITE_BCKG, BLACK_CHAR));
			m.write_text("\r\n    Now you ll be requested to particapte...");
			m.write_text("\r\n please.. DO YOUR BEST !");
			m.redirect_display(Minitel::State::STORY);
			break;
		case 3:
			m.ascii_noise(20);
			m.send(m.set_typo(MAGENTA_CHAR, BLUE_BCKG));
			m.write_text(" In order to");
			m.send(BLACK_CHAR);
			m.send(WHITE_BCKG);
			m.write_text(" register");
			m.send(m.set_typo(MAGENTA_CHAR, BLUE_BCKG));
			m.write_text(" you must follow the 42-i protocol\r");
			m.update_cursor(COLS_VIDEOTEX / 2 - 8, ROWS_VIDEOTEX / 2, 0);
			m.send(m.set_typo(WHITE_CHAR, BLACK_BCKG));
			m.write_text(" This protocols define the symbol ':' as command switching character\r");
			m.write_text(" Commands are triggered only in the main page\n");
			m.write_text(" type enter to continue...");
			break;
		case 4:
			m.write_text("Would you like to receive\r\nthe final collaborative artwork\r\nby email ? (yes/no)\r\n");
			break;
		case 5:
			m.write_text("This is all done...");
			m.write_text("What are you waiting for ?");
			break;
		case 6:
			m.ascii_noise(5);
			m.write_text("Congratulation ! you reach _level " + std::to_string(_level) + "\r\n");
			m.write_text("what do you have in mind at this particular moment ?\r\n");
			break;
		case 7:
			m.send(m.set_typo(WHITE_BCKG, BLACK_CHAR));
			m.write_text("\r\nYou have told us: \r\n");
			m.write_text(phrase);
			m.send(m.set_typo(WHITE_CHAR, BLACK_BCKG));
			m.write_text("\r\npress any key to continue...\r\n");
			break;
		case 8:
			m.update_cursor(COLS_VIDEOTEX / 2 - 8, ROWS_VIDEOTEX / 2 - 1, 0);
			m.write_text("Hope to see you soon !");
			m.ascii_noise(10);
			break;
	}
}
