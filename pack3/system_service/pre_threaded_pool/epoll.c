#include "epoll.h"

int epoll_setup(void) {
    int epoll_fd = epoll_create1(0);
    return epoll_fd;
}

int epoll_add(int epoll_fd, int fd, uint32_t events) {
    struct epoll_event epev = { .events = events, .data = { .fd = fd } };

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epev) < 0)
        return -1;
    return 0;
}

int epoll_mod(int epoll_fd, int fd, uint32_t events) {
    struct epoll_event epev = { .events = events, .data = { .fd = fd } };
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &epev) <0)
        return -1;
    return 0;
}

int epoll_del(int epoll_fd, int fd) {
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) <0)
        return -1;
    return 0;
}

void epoll_cleanup(int epoll_fd) {
    close(epoll_fd);
}