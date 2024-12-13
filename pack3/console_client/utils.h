#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <errno.h>
#include <string.h>

void* getlocaladdr(void* laddr);
char* get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen);

#endif
