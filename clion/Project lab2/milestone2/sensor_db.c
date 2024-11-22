#include "sensor_db.h"

FILE * open_db(char * filename, bool append) {
    printf("Attempting to open file: %s in %s mode\n", filename, append ? "append" : "write");
    FILE *file = fopen(filename, append ? "a" : "w");
    if (file == NULL) {
        perror("Error opening file");
    } else {
        printf("File opened successfully.\n");
    }
    return file;
}

int insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    if (f == NULL) {
        printf("Error: File pointer is NULL. Cannot insert data.\n");
        return -1;
    }

    printf("Inserting data: ID=%u, Value=%.6f, Timestamp=%ld\n", id, value, ts);
    if (fprintf(f, "%u, %.6f, %ld\n", id, value, ts) < 0) {
        perror("Error writing to file");
        return -1;
    }
    printf("Data inserted successfully.\n");
    return 0;
}

int close_db(FILE * f) {
    if (f == NULL) {
        printf("Error: File pointer is NULL. Cannot close file.\n");
        return EOF;
    }

    printf("Closing file...\n");
    int result = fclose(f);
    if (result == 0) {
        printf("File closed successfully.\n");
    } else {
        perror("Error closing file");
    }
    return result;
}
