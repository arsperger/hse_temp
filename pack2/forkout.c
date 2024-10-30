#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int out_fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd < 0) {
        perror("Failed to open out.txt");
        exit(EXIT_FAILURE);
    }

    int err_fd = open("err.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (err_fd < 0) {
        perror("Failed to open err.txt");
        close(out_fd);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        close(out_fd);
        close(err_fd);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {

        if (dup2(out_fd, STDOUT_FILENO) < 0) {
            perror("dup2 failed for stdout");
            close(out_fd);
            close(err_fd);
            exit(EXIT_FAILURE);
        }

        if (dup2(err_fd, STDERR_FILENO) < 0) {
            perror("dup2 failed for stderr");
            close(out_fd);
            close(err_fd);
            exit(EXIT_FAILURE);
        }

        close(out_fd);
        close(err_fd);

        execvp(argv[1], &argv[1]);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }

    close(out_fd);
    close(err_fd);

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        printf("child %d terminated normally with exit status=%d\n", pid, WEXITSTATUS(status));
    } else {
        printf("child %d terminated abnormally\n", pid);
    }

    return EXIT_SUCCESS;
}
