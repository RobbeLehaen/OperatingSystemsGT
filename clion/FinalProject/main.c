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

#define LOG_BUFFER_SIZE 1024

// Global variables for logging
static int log_pipe[2];
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t log_thread;

// Logging thread function
void *log_thread_func(void *arg) {
    FILE *log_file = fopen("gateway.log", "w");
    if (!log_file) {
        perror("Failed to open gateway.log");
        pthread_exit(NULL);
    }

    char buffer[LOG_BUFFER_SIZE];
    char timestamp[128];
    int seq_num = 0;

    while (1) {
        ssize_t bytes_read = read(log_pipe[0], buffer, LOG_BUFFER_SIZE - 1);
        if (bytes_read <= 0) break; // Pipe closed or error occurred

        buffer[bytes_read] = '\0'; // Null-terminate the buffer

        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

        fprintf(log_file, "%d %s %s\n", seq_num++, timestamp, buffer);
        fflush(log_file);
    }

    fclose(log_file);
    pthread_exit(NULL);
}

// Function to initialize logging
int init_logging() {
    if (pipe(log_pipe) == -1) {
        perror("Failed to create log pipe");
        return -1;
    }

    if (pthread_create(&log_thread, NULL, log_thread_func, NULL) != 0) {
        perror("Failed to create log thread");
        return -1;
    }

    return 0;
}

// Function to write log messages
void write_log(const char *message) {
    pthread_mutex_lock(&log_mutex);
    write(log_pipe[1], message, strlen(message));
    pthread_mutex_unlock(&log_mutex);
}

// Function to cleanup logging
void cleanup_logging() {
    close(log_pipe[1]); // Close write end of the pipe
    pthread_join(log_thread, NULL);
    close(log_pipe[0]); // Close read end of the pipe
}

// Shared buffer for sensor data
static sbuffer_t *shared_buffer = NULL;

void *data_manager_thread(void *arg) {
    FILE *room_sensor_map = fopen("room_sensor.map", "r");
    if (room_sensor_map == NULL) {
        perror("Error opening room_sensor.map");
        pthread_exit(NULL);
    }

    datamgr_init();
    datamgr_parse_sensor_files(room_sensor_map, NULL);
    fclose(room_sensor_map);

    while (1) {
        sensor_data_t data;
        if (sbuffer_remove(shared_buffer, &data) == SBUFFER_SUCCESS) {
            datamgr_process_data(&data);
        } else {
            write_log("Data manager: Buffer is empty, retrying...");
            sleep(1); // Wait before retrying
        }
    }

    datamgr_free();
    pthread_exit(NULL);
}

void *storage_manager_thread(void *arg) {
    sbuffer_t *shared_buffer = (sbuffer_t *)arg;

    FILE *csv_file = open_csv(false);
    if (!csv_file) {
        write_log("Storage manager: Failed to initialize");
        pthread_exit(NULL);
    }

    write_log("Storage manager: A new data.csv file has been created");

    while (1) {
        sensor_data_t data;
        if (sbuffer_remove(shared_buffer, &data) == SBUFFER_SUCCESS) {
            if (write_to_csv(csv_file, data.id, data.value, data.ts) == 0) {
                char log_msg[128];
                snprintf(log_msg, sizeof(log_msg), "Storage manager: Data from sensor %" PRIu16 " inserted successfully", data.id);
                write_log(log_msg);
            } else {
                write_log("Storage manager: Failed to insert data");
            }
        } else {
            sleep(1); // Wait before retrying
        }
    }

    write_log("Storage manager: Closing data.csv file");
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

    if (pthread_create(&storage_manager_tid, NULL, storage_manager_thread, (void *)shared_buffer) != 0) {
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
