#include "executor.h"
#include "epoll.h"

void execute_command(int client_socket, char *command, char **args) {
    int pipe_stdout[2], pipe_stderr[2];

    if (pipe(pipe_stdout) == -1 || pipe(pipe_stderr) == -1) {
        perror("Pipe creation failed");
        return;
    }

    pid_t pid = fork();
    if (pid == 0) { // Child process
        close(pipe_stdout[0]);
        close(pipe_stderr[0]);
        dup2(pipe_stdout[1], STDOUT_FILENO);
        dup2(pipe_stderr[1], STDERR_FILENO);
        close(pipe_stdout[1]);
        close(pipe_stderr[1]);

        //const char* prog1[] = { "date", "-u", NULL};

        //fprintf(stdout, "commands: %s %s %s %s", command, args[0], args[1], args[2]);

        execvp(command, args);
        //execvp(prog1[0], prog1);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) { // Parent process
        close(pipe_stdout[1]);
        close(pipe_stderr[1]);

        // epoll in action
        int epoll_fd = epoll_setup();
        if (epoll_fd == -1) {
            close(pipe_stdout[0]);
            close(pipe_stderr[0]);
            return;
        }

        //struct epoll_event ev, events[2];
        //ev.events = EPOLLIN;

        struct epoll_event events[2];
        u_int32_t event = EPOLLIN;

        if (epoll_add(epoll_fd, pipe_stdout[0], event) < 0 ) {
            perror("epoll_ctl: add stdout failed");
            close(pipe_stdout[0]);
            close(pipe_stderr[0]);
            close(epoll_fd);
            return;
        }

        if (epoll_add(epoll_fd, pipe_stderr[0], event) < 0 ) {
            perror("epoll_ctl: add stderr failed");
            close(pipe_stdout[0]);
            close(pipe_stderr[0]);
            close(epoll_fd);
            return;
        }

        /*
        ev.data.fd = pipe_stdout[0];
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_stdout[0], &ev) == -1) {
            perror("epoll_ctl: add stdout failed");
            close(pipe_stdout[0]);
            close(pipe_stderr[0]);
            close(epoll_fd);
            return;
        }

        ev.data.fd = pipe_stderr[0];
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_stderr[0], &ev) == -1) {
            perror("epoll_ctl: add stderr failed");
            close(pipe_stdout[0]);
            close(pipe_stderr[0]);
            close(epoll_fd);
            return;
        }
        */

        char buffer[BUFFER_SIZE];
        int status;

        while (1) {
            int n_fds = epoll_wait(epoll_fd, events, 2, TIMEOUT);
            if (n_fds == -1) {
                perror("epoll_wait failed");
                break;
            }

            if (n_fds == 0) { // Timeout
                kill(pid, SIGKILL);
                waitpid(pid, &status, 0);
                send(client_socket, "timeout\n", strlen("timeout\n"), 0);
                break;
            }

            for (int i = 0; i < n_fds; i++) {
                int fd = events[i].data.fd;

                // TODO: robust io
                while (read(fd, buffer, sizeof(buffer) - 1) > 0) {
                    send(client_socket, buffer, strlen(buffer), 0);
                }

                if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) {
                    perror("epoll_ctl: remove fd failed");
                }
                close(fd);
            }

            if (waitpid(pid, &status, WNOHANG) > 0) {
                break; // Child process exited
            }
        }

        close(pipe_stdout[0]);
        close(pipe_stderr[0]);
        close(epoll_fd);
    } else {
        perror("Fork failed");
    }
}
