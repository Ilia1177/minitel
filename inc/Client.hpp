#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Minitel1B_Hard.h"
enum { MAIN_PAGE = 1, GAME1 = 2 };
class Client
{
  public:
    Client(const char* device);
    Client(HardwareSerial& serial);
    Client(void);
    ~Client();
    std::string    current_page_str();
    void           init(HardwareSerial& s);
    int            index;
    HardwareSerial serial;
    Minitel*       minitel;
    int            currentPage;

  private:
};

#endif
