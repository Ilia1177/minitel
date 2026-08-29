#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#define POLL_TIMEOUT 1000

#define RED "\033[91m"
#define GREEN "\033[92m"
#define YELLOW "\033[93m"
#define BLUE "\033[94m"
#define ENDC "\033[0m"

enum error_status {
	ERR = 1,
	WARN = 2,
	INFO = 3
};

extern int g_signal;

class Server
{
  public:
    Server(void);
    ~Server(void);
    int listen(void);
    int handle_client_input(Client*);
int handle_server_command(std::string cmd);
    int add_client(const char* serial_path);

	void log(std::string str, error_status status);
    void game_one(Client*);
    int  game_one_input(Client*);
    int init_machine(Client* client);
    void main_page(Client* client);
    int  main_page_input(Client* client);

  private:
    std::vector<Client*>       _clients;
    std::vector<struct pollfd> _pfds;
    std::string                _rbuff;
};

#endif
