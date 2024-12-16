#ifndef _DATAMGR_H_
#define _DATAMGR_H_

#include <stdint.h>
#include <stdio.h>
#include "config.h"

#define DATAMGR_FAILURE -1
#define DATAMGR_SUCCESS 0

// Initialize the data manager
void datamgr_init(void);

// Parse the sensor map file
void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data);

// Free all memory used by the data manager
void datamgr_free(void);

// Process a single sensor data entry
int datamgr_process_data(sensor_data_t *data);

// Get the running average for a given sensor ID
sensor_value_t datamgr_get_avg(sensor_id_t sensor_id);

#endif /* _DATAMGR_H_ */
