#ifndef MINITEL_HPP
# define MINITEL_HPP
# include <iostream>
# include <iomanip>
# include <sstream>
# include <fstream>

# define COLS_VIDEOTEX 40
# define ROWS_VIDEOTEX 24

# define BLACK_CHAR		"\x1B\x40"
# define RED_CHAR		"\x1B\x41"
# define GREEN_CHAR		"\x1B\x42"
# define YELLOW_CHAR	"\x1B\x43"
# define BLUE_CHAR		"\x1B\x44"
# define MAGENTA_CHAR	"\x1B\x45"
# define CYAN_CHAR		"\x1B\x46"
# define WHITE_CHAR		"\x1B\x47"

# define BLACK_BCKG		"\x1B\x50"
# define RED_BCKG		"\x1B\x51"
# define GREEN_BCKG		"\x1B\x52"
# define YELLOW_BCKG	"\x1B\x53"
# define BLUE_BCKG		"\x1B\x54"
# define MAGENTA_BCKG	"\x1B\x55"
# define CYAN_BCKG		"\x1B\x56"
# define WHITE_BCKG		"\x1B\x57"

#define  BLINK_ON		"\x1B\x48"
#define  BLINK_OFF		"\x1B\x49"
#define  CLEAR			"\x1B\x0C"
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
//
// Couleur +clair -> +foncé
// blanc, jaune, cyan, vert, magenta, rouge, bleu, noir

enum MinitelMode {
    VIDEOTEX,  // 40 columns, Videotex codes
    ANSI       // 80 columns, ANSI codes
};

extern bool g_interrupt;

template <typename T> std::string fit(T value, int width, int precision = 2, char fillChar = 32) { std::ostringstream oss;
    oss << std::setfill(fillChar) << std::setw(width) << std::fixed << std::setprecision(precision) << value;
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
        ~Minitel();
        Minitel(void);

		int init(int ac, char** av);
		void recovery();
		void clear_line();
		void read();
		void listen();
		void handle_byte(unsigned char b);
		void ascii_mode();
		void mosaique_mode();
		void display_ascii_table();
		void g2_mode();
		void png_to_mosaique(const char* filename);
		void png_to_mosaique_dither(const char* filename);
		void bell();
		void roll_mode();
		void blink_on();
		void blink_off();
		void send(const std::string& text);
		void clear_screen();
		void cursor_on();
		void cursor_off();
		void cursor_at(int x, int y);
		void close();
		size_t dial_menu(const std::vector<std::string>& menu);
		
		void reset_usb_device();
		void test_char();
		void flush();
		void display_menu();
		void hazardous_collective(const std::string& input);
		void cadavre_exquis(const std::string& input);
		void handle_input();
		void exec_choice(const std::string& cmd);
		void beep();
		int configure_serial(const char* port);

		std::string get_state();
		void forty_two(const std::string& input);
		void fsend(const std::string& line, const std::string& cmd1 = "", const std::string& cmd2 = "", const std::string& cmd3 = "");

	private:
		enum class State { MENU, HAZARDOUS, STORY, FORTY2 };
		int serial_port_;
		MinitelMode mode_;
		State state_;
		std::string buffer_;
		std::string story_;

		std::ofstream storyBook_;
		bool debugMode_;


        Minitel(const Minitel& other);
        Minitel &operator=(const Minitel &other);
};

#endif

