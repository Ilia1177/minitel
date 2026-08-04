#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Minitel1B_Hard.h"

class Client
{
  public:
    Client(HardwareSerial& serial);
    Client(void);
    ~Client();
    void           init(HardwareSerial& s);
    int            index;
    HardwareSerial serial;
    Minitel*       minitel;     // Minitel takes HardwareSerial& in its constructor
    int            currentPage; // whatever state you need per client
};

#endif
