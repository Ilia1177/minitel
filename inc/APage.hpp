#ifndef APAGE_HPP
# define APAGE_HPP
# include "Minitel.hpp"

class APage
{
    public:
        virtual ~APage();
		APage(Minitel* minitel);

		virtual void display() = 0;
		virtual Minitel::State handle_input(const std::string& input) = 0;

		Minitel::State endState;

	protected:
		Minitel* _minitel;

	private:
        APage(void);
        APage(const APage& other);
        APage &operator=(const APage &other);
};

#endif

