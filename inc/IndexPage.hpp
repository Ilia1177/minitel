#ifndef INDEXPAGE_HPP
# define INDEXPAGE_HPP
# include <iostream>
# include "APage.hpp"

class IndexPage: public APage
{
    public:
        ~IndexPage();

		IndexPage(Minitel *minitel);
		void display();
		Minitel::State handle_input(const std::string& input);
	private:
        IndexPage(void);
        IndexPage(const IndexPage& other);
        IndexPage &operator=(const IndexPage &other);
};

#endif

