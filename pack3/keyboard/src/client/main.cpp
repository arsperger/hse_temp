#include <iostream>
#include "logger.hpp"

using namespace std;

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // new session
    if (setsid() < 0) {
        perror("setsid failed");
        exit(EXIT_FAILURE);
    }

    // fork again and kill parent
    pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);

    chdir("/");

    freopen("/dev/null", "r", stdin);
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);
}


int main(int argc, char **argv) {
	if (argc != 3) {
    	cerr << "Usage: " << argv[0] << " <host> <port>\n";
    	return EXIT_FAILURE;
    }

	string host = argv[1];
	if (host.empty()) {
		cerr << "Invalid host\n";
		return EXIT_FAILURE;
	}
	string port = argv[2];
	if (port.empty()) {
		cerr << "Invalid port\n";
		return EXIT_FAILURE;
	}

	daemonize();

    Logger logger(host, port);

    logger.loggerize();

	return 0;
}