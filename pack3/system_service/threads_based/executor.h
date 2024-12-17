#ifndef EXEC_H
#define EXEC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
//#include <sys/epoll.h>
//#include <errno.h>

#define TIMEOUT 5000
#define BUFFER_SIZE 1024

void execute_command(int client_socket, char *command, char **args);

//void execute_command(int client_socket, char **args);


#endif // EXEC_H