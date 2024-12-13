#include "utils.h"

void* getlocaladdr(void* getaddr) {
    struct ifaddrs *myaddrs, *ifa;
    void *in_addr;
    char* buf;

    buf = (char*)getaddr;

    if(getifaddrs(&myaddrs) != 0) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        if (!(ifa->ifa_flags & IFF_UP))
            continue;

        switch (ifa->ifa_addr->sa_family)
        {
            case AF_INET:
            {
                struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
                in_addr = &s4->sin_addr;
                break;
            }

            case AF_INET6:
                continue;

            default:
                continue;
        }

        if (!inet_ntop(ifa->ifa_addr->sa_family, in_addr, buf, INET_ADDRSTRLEN)) {
            fprintf(stderr, "%s: inet_ntop failed: %s and buf is: %s\n", ifa->ifa_name, strerror(errno), buf);
            exit(EXIT_FAILURE);
        }
    }

    freeifaddrs(myaddrs);
    return NULL;
}

char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen) {
    switch(sa->sa_family) {
        case AF_INET:
            inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
                    s, maxlen);
            break;

        case AF_INET6:
            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
                    s, maxlen);
            break;

        default:
            strncpy(s, "Unknown AF", maxlen);
            return NULL;
    }

    return s;
}