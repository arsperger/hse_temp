//#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "utils.h"

#define BUFFER_SIZE 1024
typedef struct sockaddr SA;

struct client_rsv_thread
{
    int sockid;
    char* localaddr;
};


void *receive_messages(void *arg) {
    //int sockfd = *(int *)arg;

    struct client_rsv_thread* argss = (struct client_rsv_thread*)arg;

    char buffer[BUFFER_SIZE];
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    //printf("fd is %d addrs: %s\n", argss->sockid, argss->localaddr);

    char* rsvaddr = (char*)malloc(sizeof(char) * INET_ADDRSTRLEN);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t received = recvfrom(argss->sockid, buffer, BUFFER_SIZE - 1, 0, (SA *)&sender_addr, &sender_len);
        if (received > 0) {
            rsvaddr = get_ip_str((SA*)&sender_addr, rsvaddr, INET_ADDRSTRLEN);
            buffer[received] = '\0';
            printf("Message received: %s from: %s\n", buffer, rsvaddr);
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* endptr;
    u_int16_t port = strtoul(argv[1], &endptr, 0);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }

    int sockfd;
    struct sockaddr_in broadcast_addr, receive_addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int broadcast_enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    memset(&receive_addr, 0, sizeof(receive_addr));
    receive_addr.sin_family = AF_INET;
    receive_addr.sin_port = htons(port);
    receive_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (SA *)&receive_addr, sizeof(receive_addr)) < 0) {
        perror("bind error");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(port);
    broadcast_addr.sin_addr.s_addr = INADDR_BROADCAST;

    char* localip;
    localip = (char*)malloc(sizeof(char) * INET_ADDRSTRLEN);
    getlocaladdr(localip);

    struct client_rsv_thread* args;
    args = (struct client_rsv_thread*)malloc(sizeof(struct client_rsv_thread));
    args->sockid = sockfd;
    args->localaddr = localip;

    printf("fd in main is %d and %d addrlocal: %s\n", sockfd, args->sockid, args->localaddr);

    pthread_t receiver_thread;
    if (pthread_create(&receiver_thread, NULL, receive_messages, args) != 0) {
        perror("pthread create error");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Chat client started. Type your messages below:\n");

    char message[BUFFER_SIZE];
    while (1) {
        fgets(message, BUFFER_SIZE, stdin);

        size_t len = strlen(message);
        if (len > 0 && message[len - 1] == '\n') {
            message[len - 1] = '\0';
        }

        if (sendto(sockfd, message, strlen(message), 0, (SA *)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
            perror("send message");
        }
    }

    pthread_cancel(receiver_thread);
    pthread_join(receiver_thread, NULL);
    close(sockfd);
    free(localip);
    free(args);

    return 0;
}
