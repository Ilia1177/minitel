#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "TermScreen.hpp"

#define POLL_TIMEOUT 1000

#define RED "\033[91m"
#define GREEN "\033[92m"
#define YELLOW "\033[93m"
#define BLUE "\033[94m"
#define ENDC "\033[0m"

#define ALIGN_CENTER_X 		(1 << 16)
#define ALIGN_CENTER_Y 		(2 << 16)
#define ALIGN_LEFT 			(3 << 16)
#define ALIGN_RIGHT 		(4 << 16)
#define ALIGN_TOP 			(5 << 16)
#define ALIGN_BOTTOM 		(6 << 16)

enum error_status {
	ERR = 1,
	WARN = 2,
	INFO = 3
};

extern int g_signal;
struct PfdOwner { Client* client; bool isPty; };
class Server
{
  public:
    Server(void);
    ~Server(void);
    int handle_client_input(Client*);
	int handle_server_command(std::string& cmd);
	int handle_client_command(Client* client, std::string& cmd);


	void log(std::string str, error_status status);

void system_page(Client* client);
void system_page_input(Client* client);
	void connexion_page(Client* client);
	int connexion_input(Client* client);
    void risographie_page(Client*);
    int  risographie_input(Client*);
    void main_page(Client* client);
    int  main_page_input(Client* client);

    int init_machine(Client* client);
    int add_client(const char* serial_path);
    int listen(void);

  private:
    std::vector<Client*>       _clients;
    std::vector<struct pollfd> _pfds;

	std::vector<PfdOwner> _owners; // parallel to _pfds[1:]
    std::string                _rbuff;
	void rebuildPfds();
};

std::vector<std::string> parse_command(std::string& cmd);
char appendCodepoint(std::string& input, unsigned long code);
void machine_print_infos(Client* client);
void input_box(Client* client);
int print_file(Client* client, const std::string &path);
void renderToMinitel(Minitel* m, TermScreen& screen);
#endif
