#ifndef SENSOR_DB_H
#define SENSOR_DB_H

#include <stdbool.h>
#include <stdio.h>

// Type definitions
typedef int sensor_id_t;
typedef float sensor_value_t;
typedef long sensor_ts_t;

/**
 * Open the sensor database file.
 * 
 * @param filename The name of the file to open.
 * @param append Whether to append to the file (true) or overwrite it (false).
 * @return A pointer to the opened file, or NULL on failure.
 */
FILE* open_db(char *filename, bool append);

/**
 * Insert a new sensor record into the database.
 * 
 * @param db_file The database file pointer.
 * @param id The sensor ID.
 * @param value The sensor value.
 * @param timestamp The timestamp of the record.
 * @return 0 on success, -1 on failure.
 */
int insert_sensor(FILE *db_file, sensor_id_t id, sensor_value_t value, sensor_ts_t timestamp);

/**
 * Close the sensor database file.
 * 
 * @param db_file The database file pointer.
 * @return 0 on success, -1 on failure.
 */
int close_db(FILE *db_file);

/**
 * Create the logger process.
 * 
 * @return 0 on success, -1 on failure.
 */
int create_log_process();

/**
 * Write a message to the logger process.
 * 
 * @param msg The message to log.
 * @return 0 on success, -1 on failure.
 */
int write_to_log_process(char *msg);

/**
 * End the logger process and clean up resources.
 * 
 * @return 0 on success, -1 on failure.
 */
int end_log_process();

#endif // SENSOR_DB_H
