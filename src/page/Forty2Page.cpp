#include "Forty2Page.hpp"

// Default constructor
Forty2Page::Forty2Page(Minitel* minitel): APage(minitel) {return ;}


Forty2Page::~Forty2Page(void) {}

std::string askOllama(const std::string& model, const std::string& prompt, Minitel* minitel);

void Forty2Page::display() {
	std::cout << "[PAGE] 42: \n";
	// std::cout << "\tCurrent: " << get_state(_state) << "\n";
	// std::cout << "\tto     : " << get_state(finaleState) << "\n";
	Minitel &m = *_minitel;
	m.send(CLEAR);
	m.cursorX = 1;
	m.cursorY = 1;
	m.write_text("The true value often lies beyond the result.\r\n");
	m.write_text("We might not see it at first glance...\r\n");
	m.write_text("This path must be choose.\r\n");
	m.write_text("And the journey is, perhaps, what matter the most.\r\n");
	m.write_text("Dont forget !! If you write a '/' you can ask the machine directly from the home page...");
	m.write_text("Did you see something in this cubic simulation ?\r\n");
	m.write_text("Did you even try it ?\r\n");
	m.ascii_noise(5);
	m.send(m.set_typo(WHITE_CHAR, BLACK_BCKG));


}

Minitel::State Forty2Page::handle_input(const std::string& input) {
	(void)input;
	Minitel &m = *_minitel;
	ThermalPrinter* printer = m.get_printer();
	if (!printer) {
		m.write_text("The printer is not ready... i cannot help you this time...\r\n");
	}
	if (input == "question") {
		std::string prompt = "Explain what is 42, and give the full response.";
		std::string reply = askOllama("llama3.1", prompt, _minitel);
		printer->writeText(reply);
		printer->printPNG("chou.png");
	} else {
		printer->writeText("You need to pay more attention...\r\n");
	}
	printer->dot_feed(10);
	return m.redirect_display(endState);
}
