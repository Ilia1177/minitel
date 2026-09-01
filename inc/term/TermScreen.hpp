// TermScreen.hpp
#pragma once
#include <vector>
#include <string>

struct Cell { char ch = ' '; bool reverse = false; };

class TermScreen {
public:
    TermScreen(int cols, int rows);
    void feed(const char* data, size_t len);

    // Returns list of (col,row,char) cells that changed since last render(),
    // plus the cursor's new position.
    struct Diff { int x, y, cx, cy; std::vector<std::tuple<int,int,char>> cells; };
    Diff render();
private:
    int cols;
	int rows;
    int cursorX;
	int cursorY;
    // int savedX = 0, savedY = 0;
    std::vector<std::vector<Cell>> buf, prev;

    // parser state
	enum class ParserState { Ground, EscSeen, CsiSeen };
	ParserState state; // = ParserState::Ground;
    std::string csiParams;

    void put(char c);
    void newline();
    void eraseInLine(int mode);
    void eraseInDisplay(int mode);
	void feedByte(unsigned char c);
    std::vector<int> params();
    void applyCSI(char final);
};
