#ifndef CONTACTPAGE_HPP
# define CONTACTPAGE_HPP
# include "APage.hpp"

class ContactPage: public APage
{
    public:
        ~ContactPage();
        ContactPage(Minitel* minitel);

		void display();
		Minitel::State handle_input(const std::string &input);
	private:
        ContactPage(void);
        ContactPage(const ContactPage& other);
        ContactPage &operator=(const ContactPage &other);
};

#endif

