#include "TermScreen.hpp"

TermScreen::TermScreen(int cols, int rows) : 
	cols(cols), 
	rows(rows), 
	cursorX(0),
	cursorY(0),
	buf(rows, std::vector<Cell>(cols)), 
	prev(rows, std::vector<Cell>(cols)),
	state(ParserState::Ground) {}

void TermScreen::feed(const char* data, size_t len)
{
	for (size_t i = 0; i < len; i++) 
		feedByte((unsigned char)data[i]);
}

TermScreen::Diff TermScreen::render()
{
	Diff d{cursorX, cursorY, cursorX, cursorY, {}};
	for (int y = 0; y < rows; y++)
		for (int x = 0; x < cols; x++)
			if (!(buf[y][x].ch == prev[y][x].ch)) {
				d.cells.emplace_back(x, y, buf[y][x].ch);
				prev[y][x] = buf[y][x];
			}
	return d;
}
void TermScreen::put(char c)
{
	if (cursorX >= cols) { 
		cursorX = 0; 
		newline(); 
	}
	buf[cursorY][cursorX].ch = c;
	cursorX++;
}

void TermScreen::newline()
{
	cursorY++;
	if (cursorY >= rows) {
		// scroll
		for (int y = 1; y < rows; y++) buf[y-1] = buf[y];
		buf[rows-1] = std::vector<Cell>(cols);
		cursorY = rows - 1;
	}
}

void TermScreen::eraseInLine(int mode)
{
	int from = 0, to = cols - 1;
	if (mode == 0) from = cursorX;
	else if (mode == 1) to = cursorX;
	for (int x = from; x <= to; x++) buf[cursorY][x] = Cell{};
}

void TermScreen::eraseInDisplay(int mode)
{
	if (mode == 2) { for (auto& row : buf) row = std::vector<Cell>(cols); cursorX = cursorY = 0; return; }
	// partial modes: extend as needed
}

void TermScreen::feedByte(unsigned char c)
{
	if (state == ParserState::Ground) {
		if (c == 0x1B) { state = ParserState::EscSeen; return; }
		if (c == '\r') { cursorX = 0; return; }
		if (c == '\n') { newline(); return; }
		if (c == 0x08) { if (cursorX > 0) cursorX--; return; }
		if (c == 0x07) { return; }
		if (c >= 0x20 && c < 0x7F) { put((char)c); return; }
		return;
	}
	if (state == ParserState::EscSeen) {
		if (c == '[') { state = ParserState::CsiSeen; csiParams.clear(); return; }
		state = ParserState::Ground;
		return;
	}
	if (state == ParserState::CsiSeen) {
		if ((c >= '0' && c <= '9') || c == ';') { csiParams += (char)c; return; }
		applyCSI(c);
		state = ParserState::Ground;
	}
}

std::vector<int> TermScreen::params()
{
	std::vector<int> p;
	std::string cur;
	for (char c : csiParams) {
		if (c == ';') { p.push_back(cur.empty() ? 0 : std::stoi(cur)); cur.clear(); }
		else cur += c;
	}
	p.push_back(cur.empty() ? 0 : std::stoi(cur));
	return p;
}

void TermScreen::applyCSI(char final)
{
	auto p = params();
	int n = p.empty() ? 0 : p[0];
	switch (final) {
		case 'A': cursorY = std::max(0, cursorY - std::max(n,1)); break;
		case 'B': cursorY = std::min(rows-1, cursorY + std::max(n,1)); break;
		case 'C': cursorX = std::min(cols-1, cursorX + std::max(n,1)); break;
		case 'D': cursorX = std::max(0, cursorX - std::max(n,1)); break;
		case 'H': case 'f': {
			int row = p.size() > 0 ? p[0] : 1;
			int col = p.size() > 1 ? p[1] : 1;
			cursorY = std::max(0, std::min(rows-1, row - 1));
			cursorX = std::max(0, std::min(cols-1, col - 1));
			break;
		}
		case 'J': eraseInDisplay(n); break;
		case 'K': eraseInLine(n); break;
		case 'm': /* SGR: ignore or track reverse for now */ break;
		default: break; // extend: scroll region (r), insert/delete line (L/M), etc.
	}
}
