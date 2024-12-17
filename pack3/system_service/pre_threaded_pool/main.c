#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#include "utils.h"
#include "executor.h"

#define QUEUE_SIZE 100
#define THREAD_POOL_SIZE 4

struct client_request {
    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len;
};


struct client_request request_queue[QUEUE_SIZE];
int queue_front = 0;
int queue_rear = 0;
int queue_count = 0;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

void handle_client(struct client_request *request) {
    int client_socket = request->client_socket;

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
        return;
    }

    execute_command(client_socket, args[0], args);
    close(client_socket);
}

void *thread_function(void *arg) {
    while (1) {
        pthread_mutex_lock(&queue_mutex);

        while (queue_count == 0) {
            pthread_cond_wait(&queue_cond, &queue_mutex);
        }

        struct client_request request = request_queue[queue_front];
        queue_front = (queue_front + 1) % QUEUE_SIZE;
        queue_count--;

        pthread_mutex_unlock(&queue_mutex);

        handle_client(&request);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, QUEUE_SIZE) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    pthread_t thread_pool[THREAD_POOL_SIZE];
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        if (pthread_create(&thread_pool[i], NULL, thread_function, NULL) != 0) {
            perror("Thread creation failed");
            close(server_socket);
            exit(EXIT_FAILURE);
        }
    }

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);

        if (client_socket == -1) {
            perror("accept failed");
            continue;
        }

        pthread_mutex_lock(&queue_mutex);

        if (queue_count < QUEUE_SIZE) {
            request_queue[queue_rear].client_socket = client_socket;
            request_queue[queue_rear].client_addr = client_addr;
            request_queue[queue_rear].addr_len = client_len;
            queue_rear = (queue_rear + 1) % QUEUE_SIZE;
            queue_count++;

            pthread_cond_signal(&queue_cond);
        } else {
            fprintf(stderr, "queue is full, rejecting connection\n");
            close(client_socket);
        }

        pthread_mutex_unlock(&queue_mutex);
    }

    close(server_socket);
    return 0;
}
