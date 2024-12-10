#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "connmgr.h"
#include "datamgr.h"
#include "sbuffer.h"
#include "config.h"

// Shared buffer for sensor data
static sbuffer_t *shared_buffer = NULL;

void *data_manager_thread(void *arg) {
    FILE *room_sensor_map = fopen("room_sensor.map", "r");
    if (room_sensor_map == NULL) {
        perror("Error opening room_sensor.map");
        pthread_exit(NULL);
    }

    datamgr_init();
    datamgr_parse_sensor_files(room_sensor_map, NULL); // Parse the room-sensor mapping
    fclose(room_sensor_map);

    while (1) {
        sensor_data_t data;
        if (sbuffer_remove(shared_buffer, &data) == SBUFFER_SUCCESS) {
            datamgr_process_data(&data); // Process the incoming data
        } else {
            fprintf(stderr, "No data in buffer or error occurred. Retrying...\n");
            sleep(1); // Wait before retrying
        }
    }

    datamgr_free();
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <port> <max_clients>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int max_clients = atoi(argv[2]);

    if (sbuffer_init(&shared_buffer) != SBUFFER_SUCCESS) {
        fprintf(stderr, "Failed to initialize shared buffer\n");
        exit(EXIT_FAILURE);
    }

    pthread_t data_manager_tid;
    if (pthread_create(&data_manager_tid, NULL, data_manager_thread, NULL) != 0) {
        perror("Error creating data manager thread");
        exit(EXIT_FAILURE);
    }

    if (connmgr_init(port, max_clients, shared_buffer) != 0) {
        fprintf(stderr, "Failed to initialize connection manager\n");
        exit(EXIT_FAILURE);
    }

    connmgr_listen();
    connmgr_cleanup();

    pthread_cancel(data_manager_tid);
    pthread_join(data_manager_tid, NULL);

    sbuffer_free(&shared_buffer);

    return EXIT_SUCCESS;
}
