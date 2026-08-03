#include "Client.hpp"

Client::Client(): minitel(nullptr) {}

Client::Client(HardwareSerial& s): serial(s) {
	this->minitel = new Minitel(s);
}

void Client::init(HardwareSerial& s) {
	this->minitel = new Minitel(s);
	serial = s;
}

Client::~Client() {
	if (minitel)
		delete minitel;
}
