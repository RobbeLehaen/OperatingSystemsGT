#include "sensor_db.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#define CSV_FILENAME "data.csv"

// Open the CSV file for writing
FILE* open_csv(bool append) {
    FILE *f = fopen(CSV_FILENAME, append ? "a" : "w");
    if (!f) {
        perror("Failed to open data.csv");
        return NULL;
    }

    // If truncating, add the header
    if (!append) {
        fprintf(f, "SensorID,Value,Timestamp\n");
    }

    return f;
}

// Write a sensor entry to the CSV file
int write_to_csv(FILE *csv_file, sensor_id_t id, sensor_value_t value, sensor_ts_t timestamp) {
    if (fprintf(csv_file, "%d,%.2f,%ld\n", id, value, timestamp) < 0) {
        perror("Failed to write to data.csv");
        return -1;
    }
    fflush(csv_file); // Ensure the data is flushed
    return 0;
}

// Close the CSV file
void close_csv(FILE *csv_file) {
    if (csv_file) {
        fclose(csv_file);
    }
}
