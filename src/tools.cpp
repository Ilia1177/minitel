#include <iostream>
#include "Minitel.hpp"

const double EPSILON = std::numeric_limits<double>::epsilon();

void trim(std::string& str)
{
	if (str.empty())
		return;
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        str.clear(); // All whitespace
        return;
    }

    str.erase(0, start);
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    str.erase(end + 1);
}

void eraseLines(int lines)
{
    for (int i = 0; i < lines; ++i) {
        std::cout << "\033[1A\r\033[2K";
    }
    std::cout << std::flush;
}

void clearScreen()
{
    std::cout << "\033[2J\033[H" << std::flush;
}

bool user_line(const std::string& str, double& value, bool blocking) {
	std::string line;

	value = 0.0;
	while (!g_interrupt) {
		std::cout << str;
		if (!getline(std::cin, line)) 
			return false;
		if (g_interrupt)
			return false;
		try {
			value = std::stod(line);
		} catch (std::exception& e) {
			if (blocking) {
				eraseLines(1);
				std::cerr << "Error: " << e.what() << ". ";
			}
		}
		if (blocking && value < EPSILON) {
			continue;
		} else if (value < 0.0) {
			continue;
		}
		break;
	} 
	return true;
}

bool user_line(const std::string& str, int& value, bool blocking) {
	std::string line;

	value = 0;
	while (!g_interrupt) {
		std::cout << str;
		if (!getline(std::cin, line)) 
			return false;
		if (g_interrupt)
			return false;
		try {
			value = std::stoi(line);
		} catch (std::exception& e) {
			if (blocking) {
				eraseLines(1);
				std::cerr << "Error: " << e.what() << ". ";
				continue;
			}
		}
		if (blocking && value < 0) {
			eraseLines(1);
			std::cerr << "Choice should be in menu... (" << value << "). ";
			continue;
		}
		break;
	}
	return true;
}

bool user_line(const std::string& str, std::string& input, bool blocking) {
	std::string line = "";

	while (!g_interrupt) {
		std::cout << str;
		if (!std::getline(std::cin, line))
			return false;
		if (g_interrupt)
			return false;
		trim(line);
		// if (line.length() > maxLength) {
			// eraseLines(1);
			// std::cout << maxLength << " characters max. ";
			// continue;
		if (blocking && line.empty()) {
			eraseLines(1);
			std::cout << "Field needed ! ";
			continue;
		}
		break;
	};
	input = line;
	return true;
}

