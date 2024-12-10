#include "sensor_db.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    snprintf(log_msg, sizeof(log_msg), "Data inserted");

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
