#include "sensor_db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024
static int log_pipe[2];
static pid_t logger_pid = -1;

int create_log_process() {
    if (pipe(log_pipe) == -1) {
        perror("pipe");
        return -1;
    }

    logger_pid = fork();
    if (logger_pid == -1) {
        perror("fork");
        return -1;
    }

    if (logger_pid == 0) { // Logger process
        close(log_pipe[1]);

        FILE *log_file = fopen("gateway.log", "a");
        if (!log_file) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        char timestamp[128];
        int seq_num = 0;

        while (1) {
            ssize_t bytes_read = read(log_pipe[0], buffer, BUFFER_SIZE - 1);
            if (bytes_read <= 0) {
                break;
            }

            buffer[bytes_read] = '\0'; // Null-terminate the buffer

            char *line = strtok(buffer, "\n");
            while (line != NULL) {
                time_t now = time(NULL);
                struct tm *tm_info = localtime(&now);
                strftime(timestamp, sizeof(timestamp), "%a %b %d %H:%M:%S %Y", tm_info);

                fprintf(log_file, "%d - %s - %s\n", seq_num++, timestamp, line);
                fflush(log_file);

                line = strtok(NULL, "\n");
            }
        }

        fclose(log_file);
        close(log_pipe[0]);
        exit(EXIT_SUCCESS);
    } else { // Main process
        close(log_pipe[0]);
        return 0;
    }
}

int write_to_log_process(char *msg) {
    if (logger_pid == -1) {
        printf("Error: Logger process not created.\n");
        return -1;
    }

    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s\n", msg); // Add newline as a delimiter

    size_t len = strlen(buffer);
    if (write(log_pipe[1], buffer, len) != (ssize_t)len) {
        perror("write");
        return -1;
    }

    return 0;
}

int end_log_process() {
    if (logger_pid == -1) {
        return -1;
    }

    close(log_pipe[1]);
    waitpid(logger_pid, NULL, 0);
    logger_pid = -1;
    return 0;
}

// Database functions
FILE* open_db(char *filename, bool append) {
    FILE *f = fopen(filename, append ? "a" : "w");
    if (!f) {
        perror("Failed to open sensor database");
        return NULL;
    }

    if (create_log_process() == -1) {
        printf("Error: Logger process creation failed.\n");
        fclose(f);
        return NULL;
    }

    if (write_to_log_process("Sensor database opened.") != 0) {
        fprintf(stderr, "Error writing to log process\n");
        fclose(f);
        return NULL;
    }

    return f;
}

int insert_sensor(FILE *db_file, sensor_id_t id, sensor_value_t value, sensor_ts_t timestamp) {
    fprintf(db_file, "%d,%f,%ld\n", id, value, timestamp);
    fflush(db_file); // Ensure data is flushed to the database

    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Data inserted: ID=%d, Value=%.2f, Timestamp=%ld", id, value, timestamp);

    if (write_to_log_process(log_msg) != 0) {
        fprintf(stderr, "Error writing to log process\n");
        return -1;
    }

    return 0;
}

int close_db(FILE *db_file) {
    fclose(db_file);

    if (write_to_log_process("Sensor database closed.") != 0) {
        fprintf(stderr, "Error writing to log process\n");
        return -1;
    }

    return end_log_process();
}
