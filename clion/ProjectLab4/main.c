#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "sbuffer.h"

#define SENSOR_DATA_FILE "sensor_data"
#define OUTPUT_FILE "sensor_data_out.csv"

typedef struct {
    sbuffer_t *buffer;
    int id;
} thread_args_t;

void *writer_thread(void *arg) {
    thread_args_t *args = (thread_args_t *)arg;
    sbuffer_t *buffer = args->buffer;
    FILE *input = fopen(SENSOR_DATA_FILE, "rb");
    if (!input) {
        perror("Failed to open input file");
        return NULL;
    }

    sensor_data_t data;
    while (fread(&data.id, sizeof(data.id), 1, input) &&
           fread(&data.value, sizeof(data.value), 1, input) &&
           fread(&data.ts, sizeof(data.ts), 1, input)) {
        printf("Writer: Adding sensor data id=%hu, value=%.2f, ts=%ld\n", data.id, data.value, data.ts);
        sbuffer_insert(buffer, &data);
        usleep(10000);
           }

    fclose(input);

    // Add end-of-stream marker
    data.id = 0;
    data.value = 0;
    data.ts = 0;
    sbuffer_insert(buffer, &data);
    printf("Writer: End-of-stream marker added\n");

    return NULL;
}


void *reader_thread(void *arg) {
    thread_args_t *args = (thread_args_t *)arg;
    sbuffer_t *buffer = args->buffer;
    FILE *output = fopen("sensor_data_out.csv", "a");
    if (!output) {
        perror("Failed to open output file");
        return NULL;
    }

    sensor_data_t data;
    while (1) {
        int status = sbuffer_remove(buffer, &data);
        if (status == SBUFFER_NO_DATA) {
            break; // No more data
        }
        if (data.id == 0) {
            break;
        }
        fprintf(output, "%hu,%.2f,%ld\n", data.id, data.value, data.ts);
        printf("Reader %d: Removed sensor data id=%hu, value=%.2f, ts=%ld\n",
               args->id, data.id, data.value, data.ts);
        usleep(25000);
    }

    fclose(output);
    printf("Reader %d: Finished\n", args->id);
    return NULL;
}


int main() {
    sbuffer_t *buffer;
    pthread_t writer, reader1, reader2;

    // Initialize shared buffer
    if (sbuffer_init(&buffer) != SBUFFER_SUCCESS) {
        fprintf(stderr, "Failed to initialize buffer\n");
        return EXIT_FAILURE;
    }

    // Create thread arguments
    thread_args_t writer_args = {buffer, 0};
    thread_args_t reader_args1 = {buffer, 1};
    thread_args_t reader_args2 = {buffer, 2};

    // Create threads
    pthread_create(&writer, NULL, writer_thread, &writer_args);
    printf("Writer thread started\n");

    pthread_create(&reader1, NULL, reader_thread, &reader_args1);
    printf("Reader thread 1 started\n");

    pthread_create(&reader2, NULL, reader_thread, &reader_args2);
    printf("Reader thread 2 started\n");


    // Wait for threads to finish
    pthread_join(writer, NULL);
    pthread_join(reader1, NULL);
    pthread_join(reader2, NULL);

    // Free shared buffer
    sbuffer_free(&buffer);
    printf("Main: Buffer freed and program finished\n");

    return EXIT_SUCCESS;
}
