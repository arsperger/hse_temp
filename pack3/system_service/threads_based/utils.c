#include "utils.h"

// reentrant strtok_r in action
void tokenize(char cmd[], char* args[], size_t buff_size) {
    char* saveptr1, *saveptr2, *tok, *command;
    int argc = 0;

    command = strtok_r(cmd, "\r\n", &saveptr1);
    if (!command) {
        args = NULL;
        return;
    }

    char * cm = strtok_r(command, " ", &saveptr2);
    args[argc++] = cm;

    while ( (tok = strtok_r(NULL, " ", &saveptr2)) != NULL && argc < buff_size - 1 ) {
        args[argc++] = tok;
    }
    args[argc] = NULL;

    return;
}
