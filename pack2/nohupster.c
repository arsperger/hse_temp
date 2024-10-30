#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        printf("parent process exits, pid %d\n", pid);
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0) {
        perror("setsid failed");
        exit(EXIT_FAILURE);
    }

    int dev_null = open("/dev/null", O_RDWR);
    if (dev_null < 0) {
        perror("open /dev/null failed");
        exit(EXIT_FAILURE);
    }
    dup2(dev_null, STDIN_FILENO);
    dup2(dev_null, STDOUT_FILENO);
    dup2(dev_null, STDERR_FILENO);

    close(dev_null);

    execvp(argv[1], NULL);

    perror("execvp failed");
    exit(EXIT_FAILURE);
}
