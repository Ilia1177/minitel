#ifndef MINITEL_HPP
#define MINITEL_HPP

// #include "Page.hpp"
// #include "videotex-cmd.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <termios.h>
#include <sstream>
#include <random>
#include <map>
// #include "MazePage.hpp"
// #include "CadavrePage.hpp"
// #include "ContactPage.hpp"
// #include "IndexPage.hpp"
// #include "Forty2Page.hpp"

#define COLS_VIDEOTEX 40
#define ROWS_VIDEOTEX 24

class MazePage;
class CadavrePage;
class ContactPage;
class IndexPage;
class Forty2Page;

# define BS "\x08"
# define ESC "\x1B"
# define DC3 "\x13"
# define CLS "\x0C"
# define CLEOL "\x18"
// # define G0 "\x0F"
// # define SI "\x0f"
// # define G1 "\x0E"
// # define SO "\x0e"
// # define G2 "\x19"

# define PRO1 ESC "\x39"
# define PRO2 ESC "\x3A"
# define PRO3 ESC "\x3B"

// # define SS2 (0x19)
# define MT_BS 0x08
# define MT_ESC 0x1B
# define MT_DC3 0x13

// 1.2.6.3 Fonctions d'extension de code
#define SO   "\x0E"  // Shift Out : Accès au jeu G1. => Mode semi-graphique
#define SI   "\x0F"  // Shift In : Accès au jeu G0.  => Mode alphanumérique
#define SS2  "\x19"  // Single Shift 2 : Appel d'un caractère unique du jeu G2.
#define ESC  "\x1B"  // Escape : Echappement et accès à la grille C1.
				   //
# define QUERY_CPOS "\x1B\x5B\x36\x6E"

// commandes recus
#define ENVOI       "\x13\x41"
#define RETOUR      "\x13\x42"
#define REPETITION  "\x13\x43"
#define GUIDE       "\x13\x44"
#define ANNULATION  "\x13\x45"
#define SOMMAIRE    "\x13\x46"
#define CORRECTION  "\x13\x47"
#define SUITE       "\x13\x48"
#define ARROW_UP    "\x1B\x5B\x41" // ESC [ A
#define ARROW_DOWN  "\x1B\x5B\x42" // ESC [ B
#define ARROW_RIGHT "\x1B\x5B\x43" // ESC [ C
#define ARROW_LEFT  "\x1B\x5B\x44"

// commandes emise
#define CUR_LEFT     "\x08" // Move cursor left 1 position (backspace)
#define CUR_RIGHT    "\x09" // Move cursor right 1 position (tab)
#define CUR_DOWN     "\x0A" // Move cursor down 1 line (line feed)
#define CUR_UP       "\x0B" // Move cursor up 1 line (vertical tab)
#define BLACK_CHAR   "\x1B\x40"
#define RED_CHAR     "\x1B\x41"
#define GREEN_CHAR   "\x1B\x42"
#define YELLOW_CHAR  "\x1B\x43"
#define BLUE_CHAR    "\x1B\x44"
#define MAGENTA_CHAR "\x1B\x45"
#define CYAN_CHAR    "\x1B\x46"
#define WHITE_CHAR   "\x1B\x47"

#define BLACK_BCKG   "\x1B\x50"
#define RED_BCKG     "\x1B\x51"
#define GREEN_BCKG   "\x1B\x52"
#define YELLOW_BCKG  "\x1B\x53"
#define BLUE_BCKG    "\x1B\x54"
#define MAGENTA_BCKG "\x1B\x55"
#define CYAN_BCKG    "\x1B\x56"
#define WHITE_BCKG   "\x1B\x57"

#define BLINK_ON  "\x1B\x48"
#define BLINK_OFF "\x1B\x49"
#define CLEAR     "\x1B\x0C"
// Mode MINITEL -> OPTION HARDWARE
// Fnct + T then I : memory reset
// Fnct + T then V : Standard Téletél mode Vidéotext (40 colonnes)
// Fnct + T then A : Standard "téléinformatique" ASCII US (80 colonnes)
// Fnct + T then F : Standard "téléinformatique" ASCII FR (80 colonnes)
// Fnct + T then E : Echo local (On/Off)
// Fnct + C then E : Enable extended keyboard (ctrl, esc, arrows)
// Fnct + C then V : Disabled extended keyboard
// Fnct + C then M : Minuscule/Majscule
// Fnct + P then 3 : 300 bauds
// Fnct + P then 1 : 1200 bauds
// Fnct + P then 4 : 4800 bauds
// Fnct + P then 9 : 9600 bauds (non disponible pour le minitel 1B)
// Fnct + E then R : mode rouleau
// Fnct + E then P : mode page

// Couleur +clair -> +foncé
// blanc, jaune, cyan, vert, magenta, rouge, bleu, noir
//
// Non retour d'acquitement sur prise

# define CON "\x11"
# define COFF "\x14"

#define MODE_VIDEOTEX       PRO1 "\x68"  // Enter Videotex, echo ON
#define MODE_VIDEOTEX_NOACK PRO1 "\x6B"  // Enter Videotex, echo OFF, no ACK
#define MODE_TELEINFO       PRO1 "\x59"  // Enter Teleinformatique mode
#define DISCONNECT          PRO1 "\x67"  // Disconnect (return to local mode)
#define MIXED_MODE          PRO2 "\x69\x58"  // Mixed Videotex/ANSI mode
#define TELEINFO_MODE       PRO2 "\x69\x41"  // Teleinformatique mode
#define SCROLL_ON           PRO2 "\x69\x43"  // Enable scroll (rouleau)
#define SCROLL_OFF          PRO2 "\x6A\x43"  // Disable scroll (page mode)
#define KEYBOARD_LOWER      PRO2 "\x69\x45"  // Lowercase mode
#define KEYBOARD_UPPER      PRO2 "\x6A\x45"  // Uppercase mode
#define ACK_OFF             PRO2 "\x64\x53"  // No ACK on serial port (prise)
#define ACK_ON              PRO2 "\x6B\x53"  // Enable ACK on serial port
#define SPEED_300           PRO2 "\x6B\x52"  // 300 bauds
#define SPEED_1200          PRO2 "\x6B\x64"  // 1200 bauds
#define SPEED_4800          PRO2 "\x6B\x76"  // 4800 bauds
#define SPEED_9600          PRO2 "\x6B\x7F"  // 9600 bauds (Minitel 2 only)
#define STATUS_QUERY        PRO2 "\x7B\x61"  // Query Minitel status
#define CURSOR_KEYS_C0      PRO2 "\x43"      // Cursor keys send C0 codes
#define CURSOR_KEYS_C1      PRO2 "\x44"      // Cursor keys send C1 codes
#define ECHO_OFF            PRO3 "\x60\x5A\x51"  // Disable local echo
#define ECHO_ON             PRO3 "\x61\x5A\x51"  // Enable local echo
#define KEYBOARD_EXTENDED   PRO3 "\x69\x59\x41"  // Extended keyboard (arrows work!)
#define KEYBOARD_VIDEOTEX   PRO3 "\x6A\x59\x41"  // Standard Videotex keyboard
#define FUNCTION_EXTENDED   PRO3 "\x69\x59"      // Extended function keys
#define FUNCTION_STANDARD   PRO3 "\x6A\x59"      // Standard function keys
												 //
#define INIT MIXED_MODE ACK_OFF ECHO_OFF SCROLL_ON KEYBOARD_LOWER KEYBOARD_EXTENDED CON// Videotex mode, echo OFF, no ACK
// #define INIT INIT_MIXED_MODE PRO2 "\x69\x58" "\x64" "\x51" PRO3 "\x61" // Videotex mode, echo OFF, no ACK
// #define INIT         MODE_INIT_STRING CON
enum MinitelMode {
    VIDEOTEX, // 40 columns, Videotex codes
    ANSI      // 80 columns, ANSI codes
};

enum EditionMode {
	TRUNC,
	JUSTIFY,
	CENTER,
	LEFT,
	RIGHT
};

extern bool g_interrupt;

template <typename T> std::string fit(T value, int width, int precision = 2, char fillChar = 32)
{
    std::ostringstream oss;
    oss << std::setfill(fillChar) << std::setw(width) << std::fixed << std::setprecision(precision)
        << value;
    return oss.str();
}

void eraseLines(int lines);
void clearScreen();
void trim(std::string& str);

bool user_line(const std::string& str, double& value, bool blocking = true);
bool user_line(const std::string& str, int& value, bool blocking = true);
bool user_line(const std::string& str, std::string& input, bool blocking = true);

class Minitel
{
  public:
    enum class State { MENU, HAZARDOUS, STORY, FORTY2, EMAIL };
    ~Minitel();
    Minitel(void);

    int    configure_serial(const char* port);
    void   writeByte(unsigned char b);
    void   write_text(const std::string& text, int margin = 1, EditionMode mode = TRUNC);

    void   start();
    int    init(int ac, char** av);
    void   handle_input();
    bool   handle_controle_sequence();
    void   close();

	void scrollup();
	void scrolldown();

	void	rules();
	void	update_cursor(int x, int y, int margin);
	int changeSpeed(int bauds);  // Voir p.141
    void   png_to_mosaique(const char* filename);
    void   send(const std::string& text);
    void   send_file(const std::string& path, size_t lines = 0);

    size_t dial_menu(const std::vector<std::string>& menu);

	void   cursor_to(int x, int y);
    // void test_char();
    // void flush();
	
	State redirect_input(State state, const std::string& input);
	State redirect_display(State state, bool waiting = true);
	bool edition(unsigned char ch);
	bool arrows(unsigned char ch);
    void display_menu();
	void display_dialbox();
	void ascii_noise(int amount);
	//    State index_page(State finaleState, const std::string& cmd = "");
	// State add_contact(State finaleState, const std::string& input = "");
	//    State hazardous_collective(State finaleState, const std::string& input = "");
	//    State cadavre_exquis(State returnState, const std::string& input = "");
	//    State forty_two(State finaleState, const std::string& input = "");
	std::string get_typo();
	std::string set_typo(const std::string& bck, const std::string& ch, int margin = -1);
    std::string get_state(State);

	int cursorX;
	int cursorY;
  private:

	IndexPage* index;
	MazePage* maze;
	ContactPage* contact;
	Forty2Page* forty2;
	CadavrePage* cadavre;

	std::string		_paper;
	std::string		_ink;
	int _marginX;
	int _marginY = 0;

	size_t							_bufind;
    int                        _serial_port;
    MinitelMode                _mode;
    State                      _state;
	std::string				 _rbuff;
    std::string                _buffer;
    std::string                _story;

	std::string 			_instructions;
	// std::map<Page, std::string> _book;
    std::fstream _storyBook;
    std::ofstream _contacts;
    bool          _debugMode;
	
	struct termios original_termios_;  // Save original settings

    Minitel(const Minitel& other);
    Minitel& operator=(const Minitel& other);
};

#endif
