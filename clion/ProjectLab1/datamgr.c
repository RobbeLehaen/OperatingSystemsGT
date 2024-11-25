#include "datamgr.h"
#include <inttypes.h>
#include "lib/dplist.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Global linked list to store sensor data
static dplist_t *sensor_list = NULL;

// Holds the minimum and maximum temperature values
static double min_temp = SET_MIN_TEMP;
static double max_temp = SET_MAX_TEMP;

// Function to compare two sensor IDs for the list
int element_compare(void *x, void *y) {
    sensor_data_t *sensor_x = (sensor_data_t *)x;
    sensor_data_t *sensor_y = (sensor_data_t *)y;
    if (sensor_x->id < sensor_y->id) return -1;
    if (sensor_x->id > sensor_y->id) return 1;
    return 0;
}

// Function to copy sensor data (for list insertions)
void *copy_sensor_data(void *element) {
    sensor_data_t *new_sensor = (sensor_data_t *)malloc(sizeof(sensor_data_t));
    if (new_sensor == NULL) {
        perror("Memory allocation failed for sensor data copy");
        exit(1);  // Exit or handle error as appropriate
    }

    // Copy the data
    new_sensor->id = ((sensor_data_t *)element)->id;
    new_sensor->value = ((sensor_data_t *)element)->value;
    new_sensor->ts = ((sensor_data_t *)element)->ts;

    return new_sensor;
}

// Function to free sensor data
void free_sensor_data(void **element) {
    if (element != NULL && *element != NULL) {
        free(*element);  // Free the struct
        *element = NULL;  // Null the pointer
    }
}

// Function to check if the temperature exceeds limits and log events
static void check_temperature_limits(sensor_data_t *sensor) {
    if (sensor->value < min_temp) {
        fprintf(stderr, "Error: Room ID %" PRIu16 " (Sensor ID %" PRIu16 ") is too cold! Temperature: %.2f°C\n", sensor->id, sensor->id, sensor->value);
    } else if (sensor->value > max_temp) {
        fprintf(stderr, "Error: Room ID %" PRIu16 " (Sensor ID %" PRIu16 ") is too hot! Temperature: %.2f°C\n", sensor->id, sensor->id, sensor->value);
    }
}

// Initialize the data manager and allocate memory for the sensor list
void datamgr_init() {
    fprintf(stderr, "Initializing data manager...\n");

    // Create the list
    sensor_list = dpl_create(copy_sensor_data, free_sensor_data, element_compare);
    if (sensor_list == NULL) {
        fprintf(stderr, "Failed to create sensor list.\n");
        exit(1);  // or handle the error gracefully
    }

    fprintf(stderr, "Data manager initialized successfully.\n");
}

// Parse the sensor map file and populate the sensor list
void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data) {
    fprintf(stderr, "Starting datamgr_parse_sensor_files...\n");

    ERROR_HANDLER(fp_sensor_map == NULL, "Sensor map file could not be opened");
    ERROR_HANDLER(fp_sensor_data == NULL, "Sensor data file could not be opened");

    uint16_t room_id, sensor_id;
    while (fscanf(fp_sensor_map, "%" PRIu16 " %" PRIu16 "\n", (unsigned int*)&room_id, (unsigned int*)&sensor_id) == 2) {
        sensor_data_t sensor;
        sensor.id = sensor_id;
        sensor.value = 0;  // Initial value
        sensor.ts = 0;     // No timestamp yet

        // Insert sensor data into the linked list
        dplist_t *result = dpl_insert_at_index(sensor_list, &sensor, dpl_size(sensor_list), true);
        if (result == NULL) {
            fprintf(stderr, "Failed to insert sensor with ID %" PRIu16 "\n", sensor.id);
        }
    }

    // Debug: List all sensor IDs in the list after all insertions
    fprintf(stderr, "Sensors in the list after all insertions:\n");
    for (int i = 0; i < dpl_size(sensor_list); i++) {
        sensor_data_t *sensor = (sensor_data_t *)dpl_get_element_at_index(sensor_list, i);
        fprintf(stderr, "Sensor ID in list: %" PRIu16 "\n", sensor->id);
    }
}

// Free all the memory used by the data manager
void datamgr_free() {
    fprintf(stderr, "Freeing data manager resources...\n");
    dpl_free(&sensor_list, true);
    sensor_list = NULL;
    fprintf(stderr, "Data manager resources freed successfully.\n");
}

// Get the room ID for a given sensor ID
uint16_t datamgr_get_room_id(sensor_id_t sensor_id) {
    sensor_data_t temp_sensor;
    temp_sensor.id = sensor_id;

    int index = dpl_get_index_of_element(sensor_list, &temp_sensor);
    ERROR_HANDLER(index == -1, "Sensor ID not found");
    sensor_data_t *sensor = (sensor_data_t *)dpl_get_element_at_index(sensor_list, index);

    return sensor->id;  // In this case, the sensor ID and room ID are the same
}

// Get the running average for a given sensor ID
sensor_value_t datamgr_get_avg(sensor_id_t sensor_id) {
    sensor_data_t temp_sensor;
    temp_sensor.id = sensor_id;

    int index = dpl_get_index_of_element(sensor_list, &temp_sensor);
    ERROR_HANDLER(index == -1, "Sensor ID not found");
    sensor_data_t *sensor = (sensor_data_t *)dpl_get_element_at_index(sensor_list, index);

    // If there are less than RUN_AVG_LENGTH readings, return 0
    if (dpl_size(sensor_list) < RUN_AVG_LENGTH) {
        return 0;
    }

    // Calculate the running average (assuming the list maintains the latest readings)
    double sum = 0;
    int count = 0;
    for (int i = dpl_size(sensor_list) - 1; i >= 0 && count < RUN_AVG_LENGTH; i--) {
        sensor_data_t *current_sensor = (sensor_data_t *)dpl_get_element_at_index(sensor_list, i);
        sum += current_sensor->value;
        count++;
    }

    sensor_value_t avg = sum / count;
    return avg;
}

// Get the last modified timestamp for a given sensor ID
time_t datamgr_get_last_modified(sensor_id_t sensor_id) {
    sensor_data_t temp_sensor;
    temp_sensor.id = sensor_id;

    int index = dpl_get_index_of_element(sensor_list, &temp_sensor);
    ERROR_HANDLER(index == -1, "Sensor ID not found");
    sensor_data_t *sensor = (sensor_data_t *)dpl_get_element_at_index(sensor_list, index);

    return sensor->ts;
}
