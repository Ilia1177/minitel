#include "APage.hpp"
// Default constructor

APage::APage(Minitel* minitel): endState(Minitel::State::MENU), _minitel(minitel) {
	if (!minitel) 
		throw std::logic_error("Invalid minitel");
}
APage::~APage() {}
