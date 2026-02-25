#ifndef PAGE_HPP
# define PAGE_HPP
# include <iostream>
# include <map>

class Box
{
	public:
		int h;
		int w;
		int x;
		int y;
};

class Page
{
    public:
        ~Page();
        Page(void);

	private:
		Box input;
        Page(const Page& other);
        Page &operator=(const Page &other);
};

#endif

