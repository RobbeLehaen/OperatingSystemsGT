#ifndef _SENSOR_DB_H_
#define _SENSOR_DB_H_

#include "config.h"
#include <stdbool.h>
#include <stdio.h>

// Opens the CSV file
// If `append` is true, opens the file in append mode; otherwise, truncates it.
FILE *open_csv(bool append);

// Writes a sensor reading to the CSV file
// Returns 0 on success, -1 on failure.
int write_to_csv(FILE *csv_file, sensor_id_t id, sensor_value_t value, sensor_ts_t timestamp);

// Closes the CSV file
void close_csv(FILE *csv_file);

#endif /* _SENSOR_DB_H_ */
