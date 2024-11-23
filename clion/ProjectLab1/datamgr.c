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
static int compare_sensor_ids(void *x, void *y) {
    sensor_data_t *sensor_x = (sensor_data_t *)x;
    sensor_data_t *sensor_y = (sensor_data_t *)y;
    if (sensor_x->id < sensor_y->id) return -1;
    if (sensor_x->id > sensor_y->id) return 1;
    return 0;
}

// Function to copy sensor data (for list insertions)
static void *copy_sensor_data(void *element) {
    sensor_data_t *new_sensor = (sensor_data_t *)malloc(sizeof(sensor_data_t));
    ERROR_HANDLER(new_sensor == NULL, "Memory allocation failed for sensor data copy");

    *new_sensor = *((sensor_data_t *)element);  // Copy the data
    return new_sensor;
}

// Function to free sensor data
static void free_sensor_data(void **element) {
    free(*element);
    *element = NULL;
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
    sensor_list = dpl_create(copy_sensor_data, free_sensor_data, compare_sensor_ids);
    if (sensor_list == NULL) {
        fprintf(stderr, "Failed to create sensor list.\n");
        exit(1);  // or handle the error gracefully
    }

    fprintf(stderr, "Data manager initialized successfully.\n");

    // Debug: Print initial list size
    fprintf(stderr, "List size after initialization: %d\n", dpl_size(sensor_list));
}

// Parse the sensor map file and populate the sensor list
void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data) {
    fprintf(stderr, "Starting datamgr_parse_sensor_files...\n");

    ERROR_HANDLER(fp_sensor_map == NULL, "Sensor map file could not be opened");
    ERROR_HANDLER(fp_sensor_data == NULL, "Sensor data file could not be opened");

    fprintf(stderr, "Sensor map and data files opened successfully.\n");

    uint16_t room_id, sensor_id;
    while (fscanf(fp_sensor_map, "%" PRIu16 " %" PRIu16 "\n", (unsigned int*)&room_id, (unsigned int*)&sensor_id) == 2) {
        fprintf(stderr, "Read room %u, sensor %u\n", room_id, sensor_id);

        sensor_data_t sensor;
        sensor.id = sensor_id;
        sensor.value = 0;  // Initial value
        sensor.ts = 0;     // No timestamp yet

        // Insert sensor data into the linked list
        dpl_insert_at_index(sensor_list, &sensor, dpl_size(sensor_list), true);

        // Debug print to check the list size after insertion
        fprintf(stderr, "List size after insertion: %d\n", dpl_size(sensor_list));
    }

    // Debug: List all sensor IDs in the list after all insertions
    fprintf(stderr, "Sensors in the list after all insertions:\n");
    for (int i = 0; i < dpl_size(sensor_list); i++) {
        sensor_data_t *sensor = (sensor_data_t *)dpl_get_element_at_index(sensor_list, i);
        fprintf(stderr, "Sensor ID in list: %" PRIu16 "\n", sensor->id);
    }

    // Now read sensor data and update the list
    time_t timestamp;
    double temperature;
    while (fread(&sensor_id, sizeof(sensor_id), 1, fp_sensor_data) == 1) {
        fprintf(stderr, "Reading sensor data for sensor %" PRIu16 "\n", sensor_id);

        fread(&temperature, sizeof(temperature), 1, fp_sensor_data);
        fread(&timestamp, sizeof(timestamp), 1, fp_sensor_data);

        sensor_data_t temp_sensor;
        temp_sensor.id = sensor_id;
        temp_sensor.value = temperature;
        temp_sensor.ts = timestamp;

        // Debug: Look for the sensor in the list
        fprintf(stderr, "Looking for sensor ID: %" PRIu16 "\n", sensor_id);

        // Check if the sensor exists in the list
        int index = dpl_get_index_of_element(sensor_list, &temp_sensor);
        if (index >= 0) {
            sensor_data_t *existing_sensor = (sensor_data_t *)dpl_get_element_at_index(sensor_list, index);
            existing_sensor->value = temperature;
            existing_sensor->ts = timestamp;
            check_temperature_limits(existing_sensor);
        } else {
            fprintf(stderr, "Error: Sensor ID %" PRIu16 " not found in the list.\n", sensor_id);
        }
    }

    fprintf(stderr, "Completed reading sensor data and updating list.\n");
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
    fprintf(stderr, "Getting room ID for sensor ID %" PRIu16 "\n", sensor_id);

    sensor_data_t temp_sensor;
    temp_sensor.id = sensor_id;

    int index = dpl_get_index_of_element(sensor_list, &temp_sensor);
    ERROR_HANDLER(index == -1, "Sensor ID not found");
    sensor_data_t *sensor = (sensor_data_t *)dpl_get_element_at_index(sensor_list, index);

    fprintf(stderr, "Room ID for sensor ID %" PRIu16 " is %" PRIu16 "\n", sensor_id, sensor->id);
    return sensor->id;  // In this case, the sensor ID and room ID are the same
}

// Get the running average for a given sensor ID
sensor_value_t datamgr_get_avg(sensor_id_t sensor_id) {
    fprintf(stderr, "Getting running average for sensor ID %" PRIu16 "\n", sensor_id);

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
    fprintf(stderr, "Running average for sensor ID %" PRIu16 " is %.2f\n", sensor_id, avg);
    return avg;
}

// Get the last modified timestamp for a given sensor ID
time_t datamgr_get_last_modified(sensor_id_t sensor_id) {
    fprintf(stderr, "Getting last modified timestamp for sensor ID %" PRIu16 "\n", sensor_id);

    sensor_data_t temp_sensor;
    temp_sensor.id = sensor_id;

    int index = dpl_get_index_of_element(sensor_list, &temp_sensor);
    ERROR_HANDLER(index == -1, "Sensor ID not found");
    sensor_data_t *sensor = (sensor_data_t *)dpl_get_element_at_index(sensor_list, index);

    fprintf(stderr, "Last modified timestamp for sensor ID %" PRIu16 " is %ld\n", sensor_id, (long)sensor->ts);
    return sensor->ts;
}

// Get the total number of unique sensors
int datamgr_get_total_sensors() {
    fprintf(stderr, "Getting total number of sensors...\n");
    int total_sensors = dpl_size(sensor_list);
    fprintf(stderr, "Total number of sensors: %d\n", total_sensors);
    return total_sensors;
}
