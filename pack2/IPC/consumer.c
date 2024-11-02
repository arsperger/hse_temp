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
        fprintf(stderr, "Usage: %s <output_directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *output_dir = argv[1];

    sem_t *sem_producer = sem_open(SEM_PRODUCER, O_CREAT, 0644, 1);
    sem_t *sem_consumer = sem_open(SEM_CONSUMER, O_CREAT, 0644, 0);

    if (sem_producer == SEM_FAILED || sem_consumer == SEM_FAILED) {
        perror("Failed to open semaphores");
        exit(EXIT_FAILURE);
    }

    int shm_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0644);
    if (shm_fd < 0) {
        perror("Error open shmem");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(shm_fd, SHM_SIZE) < 0) {
        perror("Error ftruncate");
        exit(EXIT_FAILURE);
    }

    SharedMemorySegment *shared_mem = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("Failed to map shared memory");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("wait sem consumer\n");
        sem_wait(sem_consumer);
        printf("wait sem consumer go ahead\n");

        if (shared_mem->size == 0) {
            printf("shmem is zero, exit");
            break;
        }

        char output_path[512];
        snprintf(output_path, sizeof(output_path), "%s/%s", output_dir, shared_mem->filename);
        printf("current tranferring file is %s\n", output_path);

        FILE *file = fopen(output_path, "wb");
        if (file == NULL) {
            perror("Failed to create output file");
            sem_post(sem_producer);
            continue;
        }

        size_t written = fwrite(shared_mem->buffer, 1, BUFFER_SIZE, file);
        printf("written are %ld bytes\n", written);

        fclose(file);
        sem_post(sem_producer);
    }

    printf("Exit and clean res\n");

    munmap(shared_mem, SHM_SIZE);
    sem_close(sem_producer);
    sem_close(sem_consumer);

    shm_unlink(SHARED_MEMORY_NAME);
    sem_unlink(SEM_PRODUCER);
    sem_unlink(SEM_CONSUMER);

    return 0;
}
