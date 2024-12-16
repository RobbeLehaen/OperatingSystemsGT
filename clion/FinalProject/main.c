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
#define MAX_SENSORS 1000

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

    while (1) {
        sensor_data_t *data;
        int result = sbuffer_peek(shared_buffer, &data);

        if (result == SBUFFER_SUCCESS) {
            if (data->processed == 0) {
                if (datamgr_process_data(data) == 0) {
                    data->processed = 1;
                }
            }
        } else if (result == SBUFFER_EMPTY) {
            nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = 100000000}, NULL);
        } else {
            write_log("Data manager: Failed to peek data.");
            nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = 100000000}, NULL);
        }
    }
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

    while (1) {
        sensor_data_t *data;
        int result = sbuffer_peek(shared_buffer, &data);

        if (result == SBUFFER_SUCCESS) {
            if (data->processed == 1) {
                if (write_to_csv(csv_file, data->id, data->value, data->ts) == -1) {
                    write_log("Storage manager: Failed to write data to CSV.");
                }

                if (sbuffer_remove(shared_buffer, data) == SBUFFER_FAILURE) {
                    write_log("Storage manager: Failed to remove data from buffer.");
                }
            } else {
                nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = 100000000}, NULL);
            }
        } else if (result == SBUFFER_EMPTY) {
            nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = 100000000}, NULL);
        } else {
            write_log("Storage manager: Failed to peek data from buffer due to an unexpected error.");
            nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = 100000000}, NULL);
        }
    }
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
    connmgr_cleanup();

    write_log("Server shutting down");

    pthread_cancel(data_manager_tid);
    pthread_cancel(storage_manager_tid);
    pthread_join(data_manager_tid, NULL);
    pthread_join(storage_manager_tid, NULL);

    cleanup_logging();
    sbuffer_free(&shared_buffer);

    return EXIT_SUCCESS;
}
