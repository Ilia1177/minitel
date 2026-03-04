#ifndef FORTY2PAGE_HPP
# define FORTY2PAGE_HPP
# include "APage.hpp"

class Forty2Page: public APage
{
    public:
        ~Forty2Page();
        Forty2Page(Minitel* minitel);

		void display();
		Minitel::State handle_input(const std::string& input);
	private:
        Forty2Page(const Forty2Page& other);
        Forty2Page &operator=(const Forty2Page &other);
};

#endif

