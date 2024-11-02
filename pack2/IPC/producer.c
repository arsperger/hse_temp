#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <semaphore.h>
#include <dirent.h>

#define SHARED_MEMORY_NAME "/shared_memory3"
#define SEM_PRODUCER "/sem_producer5"
#define SEM_CONSUMER "/sem_consumer5"
#define BUFFER_SIZE 2048
#define SHM_SIZE (BUFFER_SIZE + 256)

typedef struct {
    char filename[256];
    char buffer[BUFFER_SIZE];
    size_t size;
} SharedMemorySegment;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *input_dir = argv[1];

    sem_t *sem_producer = sem_open(SEM_PRODUCER, 0);
    sem_t *sem_consumer = sem_open(SEM_CONSUMER, 0);

    if (sem_producer == SEM_FAILED || sem_consumer == SEM_FAILED) {
        perror("Failed to open semaphores");
        exit(EXIT_FAILURE);
    }

    int shm_fd = shm_open(SHARED_MEMORY_NAME, O_RDWR, 0644);
    if (shm_fd < 0) {
        perror("Error opening shared mem");
        exit(EXIT_FAILURE);
    }

    SharedMemorySegment *shared_mem = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("Failed to map shared memory");
        exit(EXIT_FAILURE);
    }

    DIR *dir = opendir(input_dir);
    if (dir == NULL) {
        perror("Failed to open directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {

            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", input_dir, entry->d_name);

            FILE *file = fopen(filepath, "rb");
            if (file == NULL) {
                perror("Failed to open file");
                continue;
            }

            printf("reading file %s\n", filepath);

            size_t read_size = fread(shared_mem->buffer, 1, BUFFER_SIZE, file);
            if (read_size < 0) {
                perror("error read file");
            }
            fclose(file);

            //printf("wait prod\n");
            sem_wait(sem_producer);
            strncpy(shared_mem->filename, entry->d_name, sizeof(shared_mem->filename));
            shared_mem->size = read_size;
            printf("post cons\n");
            sem_post(sem_consumer);
        }

    }

    shared_mem->size = 0;
    sem_post(sem_consumer);

    printf("Exit and close res\n");

    closedir(dir);
    munmap(shared_mem, SHM_SIZE);
    //shm_unlink(SHARED_MEMORY_NAME);
    sem_close(sem_producer);
    sem_close(sem_consumer);
    //sem_unlink(SEM_PRODUCER);
    //sem_unlink(SEM_CONSUMER);

    return 0;
}
