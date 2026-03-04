#ifndef CADAVREPAGE_HPP
# define CADAVREPAGE_HPP
#include "APage.hpp"

class CadavrePage: public APage
{
    public:
        ~CadavrePage();
        CadavrePage(Minitel* minitel);

		void display();
		Minitel::State handle_input(const std::string& input);
	private:
        CadavrePage(void);
        CadavrePage(const CadavrePage& other);
        CadavrePage &operator=(const CadavrePage &other);
};

#endif

