#ifndef EPOLL_H
#define EPOLL_H

#include <sys/types.h>
#include <sys/epoll.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>

int epoll_setup(void);
void epoll_cleanup(int epoll_fd);

int epoll_add(int epoll_fd, int fd, uint32_t events);
int epoll_mod(int epoll_fd, int fd, uint32_t events);
int epoll_del(int epoll_fd, int fd);

#endif /* EPOLL_H */