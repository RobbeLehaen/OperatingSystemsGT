#include "sensor_db.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE 1024

static int log_pipe[2];
static pid_t logger_pid = -1;

FILE* open_db(char *filename, bool append) {
    // Open the sensor database file
    FILE *f = fopen(filename, append ? "a" : "w");
    if (!f) {
        perror("Failed to open sensor database");
        return NULL;
    }

    // Automatically create the logger process on the first open
    if (logger_pid == -1) {
        int result = create_log_process();
        if (result == -1) {
            printf("Error: Logger process creation failed.\n");
        }
    }

    return f;
}

int insert_sensor(FILE *f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    // Insert sensor data into the database
    fprintf(f, "%d, %.6f, %ld\n", id, value, ts);
    printf("Data inserted: ID=%d, Value=%.6f, Timestamp=%ld\n", id, value, ts);

    // Send a log message about the insertion event
    char log_msg[BUFFER_SIZE];
    snprintf(log_msg, sizeof(log_msg), "Data inserted: ID=%d, Value=%.6f, Timestamp=%ld", id, value, ts);
    write_to_log_process(log_msg);

    return 0;
}

int close_db(FILE *f) {
    fclose(f);

    // Send a log message about the database closure
    write_to_log_process("Sensor database closed.");

    // Clean up the logger process
    return end_log_process();
}
