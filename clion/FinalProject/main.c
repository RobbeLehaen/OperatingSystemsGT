#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <inttypes.h>
#include "connmgr.h"
#include "datamgr.h"
#include "sbuffer.h"
#include "sensor_db.h"
#include "config.h"

// Shared buffer for sensor data
static sbuffer_t *shared_buffer = NULL;
static pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
#define MAX_SENSORS 1000
static volatile int program_running = 1;

void *data_manager_thread(void *arg) {
    if (shared_buffer == NULL) {
        write_log("Data manager: Received NULL buffer pointer. Exiting thread.");
        pthread_exit(NULL);
    }

    datamgr_init();
    FILE *room_sensor_map = fopen("room_sensor.map", "r");
    if (room_sensor_map == NULL) {
        write_log("Data manager: Failed to open room_sensor.map. Exiting.");
        datamgr_free();
        pthread_exit(NULL);
    }

    datamgr_parse_sensor_files(room_sensor_map, NULL);
    fclose(room_sensor_map);

    while (program_running) {
        sensor_data_t *data = NULL;
        pthread_mutex_lock(&buffer_mutex);
        int result = sbuffer_peek(shared_buffer, &data);
        if (result == SBUFFER_SUCCESS && data) {
            if (data->processed == 0) {
                if (datamgr_process_data(data) == DATAMGR_SUCCESS) {
                    data->processed = 1;
                }
            }
        }
        pthread_mutex_unlock(&buffer_mutex);

        if (result == SBUFFER_EMPTY) {
            nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = 100000000}, NULL);
        } else if (result != SBUFFER_SUCCESS) {
            write_log("Data manager: Failed to peek data.");
            nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = 100000000}, NULL);
        }
    }
    pthread_exit(NULL);
}

void *storage_manager_thread(void *arg) {
    if (shared_buffer == NULL) {
        write_log("Storage manager: Received NULL buffer pointer. Exiting thread.");
        pthread_exit(NULL);
    }

    FILE *csv_file = open_csv(false);
    if (!csv_file) {
        write_log("Storage manager: Failed to initialize CSV file.");
        pthread_exit(NULL);
    }

    write_log("Storage manager: A new data.csv file has been created.");

    while (program_running) {
        sensor_data_t *data = NULL;
        pthread_mutex_lock(&buffer_mutex);
        int result = sbuffer_peek(shared_buffer, &data);
        if (result == SBUFFER_SUCCESS && data) {
            if (data->processed == 1) { // Processed by data manager
                if (write_to_csv(csv_file, data->id, data->value, data->ts) == 0) {
                    data->processed = 2;
                }
            }
            if (data->processed == 2) {
                if (sbuffer_remove(shared_buffer, &data) == SBUFFER_SUCCESS) {
                    free(data);
                    data = NULL;
                } else {
                    write_log("Storage manager: Failed to remove data from buffer.");
                }
            }
        }
        pthread_mutex_unlock(&buffer_mutex);

        if (result == SBUFFER_EMPTY) {
            nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = 100000000}, NULL);
        } else if (result != SBUFFER_SUCCESS) {
            write_log("Storage manager: Unexpected error.");
            nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = 100000000}, NULL);
        }
    }
    close_csv(csv_file);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <port> <max_clients>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int max_clients = atoi(argv[2]);

    if (init_logging() != 0) {
        fprintf(stderr, "Failed to initialize logging\n");
        exit(EXIT_FAILURE);
    }

    write_log("Server started");

    if (sbuffer_init(&shared_buffer) != SBUFFER_SUCCESS) {
        fprintf(stderr, "Failed to initialize shared buffer\n");
        exit(EXIT_FAILURE);
    }

    pthread_t data_manager_tid, storage_manager_tid;

    if (pthread_create(&data_manager_tid, NULL, data_manager_thread, NULL) != 0) {
        perror("Error creating data manager thread");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&storage_manager_tid, NULL, storage_manager_thread, NULL) != 0) {
        perror("Error creating storage manager thread");
        exit(EXIT_FAILURE);
    }

    if (connmgr_init(port, max_clients, shared_buffer) != 0) {
        fprintf(stderr, "Failed to initialize connection manager\n");
        exit(EXIT_FAILURE);
    }

    connmgr_listen();

    write_log("Server shutting down");
    connmgr_cleanup();

    datamgr_free();

    program_running = 0;

    pthread_join(data_manager_tid, NULL);
    pthread_join(storage_manager_tid, NULL);

    cleanup_logging();
    sbuffer_free(&shared_buffer);
    fprintf(stderr, "Server shutdown complete\n");
    return EXIT_SUCCESS;
}
