#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include "utils.h"
#include "executor.h"

typedef struct {
    int client_socket;
} client_request_t;

void *handle_client(void *arg) {
    client_request_t *request = (client_request_t *)arg;
    int client_socket = request->client_socket;
    free(request);

    // TODO: robust read from socket
    char buffer[BUFFER_SIZE];
    ssize_t rb = recv(client_socket, buffer, BUFFER_SIZE, 0);

    printf("buffer read: %ld\n", rb);

    //char* args[BUFFER_SIZE];
    char** args = (char**)malloc(sizeof(char) * BUFFER_SIZE);

    tokenize(buffer, args, BUFFER_SIZE);
    if (args[0] == NULL) {
        send(client_socket, "Invalid command\n", strlen("Invalid command\n"), 0);
        close(client_socket);
        free(args);
        return NULL;
    }

    execute_command(client_socket, args[0], args);
    close(client_socket);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);

        if (client_socket == -1) {
            perror("Accept failed");
            continue;
        }

        client_request_t *request = malloc(sizeof(client_request_t));
        request->client_socket = client_socket;

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, request);
        pthread_detach(thread_id);
    }

    close(server_socket);
    return 0;
}
