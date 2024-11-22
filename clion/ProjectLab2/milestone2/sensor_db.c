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

    // Log the "Sensor database opened" message when the database is first opened
    if (write_to_log_process("Sensor database opened.") != 0) {
        fprintf(stderr, "Error writing to log process\n");
        fclose(f);  // Close the file if logging fails
        return NULL;
    }

    return f;
}

int insert_sensor(FILE *db_file, sensor_id_t id, sensor_value_t value, sensor_ts_t timestamp) {
    // Insert the sensor data into the database file
    fprintf(db_file, "%d,%f,%ld\n", id, value, timestamp);

    // Generate a separate log message for this insertion event
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Data inserted\n");  // Ensure newline at the end

    // Send the log message to the log process
    if (write_to_log_process(log_msg) != 0) {
        fprintf(stderr, "Error writing to log process\n");
        return -1; // Return error if log process writing fails
    }

    return 0;
}

int close_db(FILE *f) {
    fclose(f);

    // Send a log message about the database closure
    write_to_log_process("Sensor database closed.");

    // Clean up the logger process
    return end_log_process();
}
