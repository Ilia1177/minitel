#include "APage.hpp"
#include <exception>
// Default constructor
APage::APage(Minitel* minitel): _minitel(minitel) {
	if (!minitel) 
		throw std::logic_error("Invalid minitel");
}
APage::~APage() {}  // ← needs to exist, even if empty
