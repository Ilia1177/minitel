#ifndef MAZEPAGE_HPP
# define MAZEPAGE_HPP
# include <iostream>
# include "APage.hpp"

class MazePage: public APage
{
    public:
        ~MazePage();
		MazePage(Minitel* minitel);

		void display();
		Minitel::State handle_input(const std::string& input);
		void reset();

	private:
		std::string phrase;
		int _level;
        MazePage(void);
        MazePage(const MazePage& other);
        MazePage &operator=(const MazePage &other);
};

#endif

