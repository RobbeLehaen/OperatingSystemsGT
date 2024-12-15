#include "sensor_db.h"
#include "connmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>


#define CSV_FILENAME "data.csv"

FILE* open_csv(bool append) {
    FILE *f = fopen(CSV_FILENAME, append ? "a" : "w");
    if (!f) {
        perror("Failed to open data.csv");
        return NULL;
    }

    if (!append) {
        fprintf(f, "SensorID,Value,Timestamp\n");
        write_log("A new data.csv file has been created.");
    }

    return f;
}

int write_to_csv(FILE *csv_file, sensor_id_t id, sensor_value_t value, sensor_ts_t timestamp) {
    if (fprintf(csv_file, "%d,%.2f,%ld\n", id, value, timestamp) < 0) {
        perror("Failed to write to data.csv");
        return -1;
    }
    fflush(csv_file);

    char log_msg[128];
    snprintf(log_msg, sizeof(log_msg), "Data insertion from sensor %" PRIu16, id);
    write_log(log_msg);

    return 0;
}

void close_csv(FILE *csv_file) {
    if (csv_file) {
        fclose(csv_file);
        write_log("The data.csv file has been closed.");
    }
}
