#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define MAGIC_SIZE 3
unsigned char dummy_magic[MAGIC_SIZE] = {0x00, 0x00, 0x00};
unsigned char original_magic[MAGIC_SIZE];

void corrupt_magic(const char *filename) {

    FILE *file = fopen(filename, "rb+");
    if (!file) {
        perror("Error opening file");
        return;
    }

    if (fread(original_magic, 1, MAGIC_SIZE, file) != MAGIC_SIZE) {
        perror("Error reading orig magic");
        fclose(file);
        return;
    }

    fseek(file, 0, SEEK_SET);
    if (fwrite(dummy_magic, 1, MAGIC_SIZE, file) != MAGIC_SIZE) {
        perror("error writing dummy");
        fclose(file);
        return;
    }
    printf("file corrupted\n");

    // save for later
    fseek(file, 0, SEEK_END);
    if (fwrite(original_magic, 1, MAGIC_SIZE, file) != MAGIC_SIZE) {
        perror("error save original magic number");
        fclose(file);
        return;
    }

    fclose(file);
}


void restore_magic(const char *filename) {

    FILE *file = fopen(filename, "rb+");
    if (!file) {
        perror("Error opening file");
        return;
    }

    long fz;
    fseek(file, 0, SEEK_END);
    fz = ftell(file);

    fseek(file, -3, SEEK_END);
    if (fread(original_magic, 1, MAGIC_SIZE, file) != MAGIC_SIZE) {
        perror("Error reading orig magic");
        fclose(file);
        return;
    }

    fseek(file, 0, SEEK_SET);
    if (fwrite(original_magic, 1, MAGIC_SIZE, file) != MAGIC_SIZE) {
        perror("Error restoring original magic number");
        fclose(file);
        return;
    }
    printf("file restored\n");

    if (truncate(filename, (fz - MAGIC_SIZE)) != 0) {
        perror("Error truncate file");
        fclose(file);
        return;
    }

    fclose(file);
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage: %s <corrupt|restore> <filename>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "corrupt") == 0) {
        corrupt_magic(argv[2]);
    } else if (strcmp(argv[1], "restore") == 0) {
        restore_magic(argv[2]);
    } else {
        fprintf(stderr, "Invalid option. Use 'corrupt' or 'restore'.\n");
        return 1;
    }

    return 0;
}
