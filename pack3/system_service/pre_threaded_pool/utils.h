#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stdio.h>
#include <unistd.h>

// reentrant strtok_r in action
void tokenize(char cmd[], char* args[], size_t buff_size);

#endif // UTILS_H