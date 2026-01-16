#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <width> <height> <output_filename>\n", argv[0]);
        return 1;
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    const char *filename = argv[3];

    if (width <= 0 || height <= 0) {
        fprintf(stderr, "Error: Width and height must be positive integers.\n");
        return 1;
    }

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Error opening file for writing");
        return 1;
    }

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    srand(ts.tv_nsec);

    size_t num_pixels = (size_t)width * height;
    size_t buffer_size = num_pixels * 4; // RGBA
    uint8_t *buffer = malloc(buffer_size);

    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        fclose(fp);
        return 1;
    }

    // Fill buffer with random bytes
    for (size_t i = 0; i < buffer_size; i++) {
        buffer[i] = rand() % 256;
    }

    size_t written = fwrite(buffer, 1, buffer_size, fp);
    if (written != buffer_size) {
        fprintf(stderr, "Error: Failed to write all data to file.\n");
        free(buffer);
        fclose(fp);
        return 1;
    }

    printf("Generated %dx%d RGBA image to %s (%lu bytes)\n", width, height, filename, buffer_size);

    free(buffer);
    fclose(fp);
    return 0;
}
