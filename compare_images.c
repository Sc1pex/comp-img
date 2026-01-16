#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

typedef struct {
    uint32_t *img1;
    uint32_t *img2;
    size_t start_pixel;
    size_t end_pixel;
    unsigned long long diff_count;
} ThreadData;

void *compare_pixels(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->diff_count = 0;

    for (size_t i = data->start_pixel; i < data->end_pixel; i++) {
        if (data->img1[i] != data->img2[i]) {
            data->diff_count++;
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Usage: %s <image1> <image2> <width> <height> <num_threads>\n", argv[0]);
        return 1;
    }

    const char *file1 = argv[1];
    const char *file2 = argv[2];
    int width = atoi(argv[3]);
    int height = atoi(argv[4]);
    int num_threads = atoi(argv[5]);

    if (width <= 0 || height <= 0 || num_threads <= 0) {
        fprintf(stderr, "Error: Width, height, and number of threads must be positive.\n");
        return 1;
    }

    size_t num_pixels = (size_t)width * height;
    size_t file_size = num_pixels * 4;

    int fd1 = open(file1, O_RDONLY);
    int fd2 = open(file2, O_RDONLY);

    if (fd1 == -1 || fd2 == -1) {
        perror("Error opening input files");
        if (fd1 != -1) close(fd1);
        if (fd2 != -1) close(fd2);
        return 1;
    }

    // Check file sizes
    struct stat st1, st2;
    fstat(fd1, &st1);
    fstat(fd2, &st2);

    if (st1.st_size != file_size || st2.st_size != file_size) {
        fprintf(stderr, "Error: File sizes do not match expected dimensions (%dx%d -> %lu bytes).\n", width, height, file_size);
        fprintf(stderr, "File1: %ld bytes, File2: %ld bytes\n", st1.st_size, st2.st_size);
        close(fd1);
        close(fd2);
        return 1;
    }

    // Memory map files
    uint32_t *img1 = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd1, 0);
    uint32_t *img2 = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd2, 0);

    if (img1 == MAP_FAILED || img2 == MAP_FAILED) {
        perror("mmap failed");
        close(fd1);
        close(fd2);
        return 1;
    }

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    size_t pixels_per_thread = num_pixels / num_threads;
    size_t remaining_pixels = num_pixels % num_threads;
    size_t current_pixel = 0;

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].img1 = img1;
        thread_data[i].img2 = img2;
        thread_data[i].start_pixel = current_pixel;
        
        size_t count = pixels_per_thread + (i < remaining_pixels ? 1 : 0);
        thread_data[i].end_pixel = current_pixel + count;
        current_pixel += count;

        if (pthread_create(&threads[i], NULL, compare_pixels, &thread_data[i]) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    unsigned long long total_diff = 0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        total_diff += thread_data[i].diff_count;
    }

    printf("Different pixels: %llu\n", total_diff);

    munmap(img1, file_size);
    munmap(img2, file_size);
    close(fd1);
    close(fd2);

    return 0;
}
