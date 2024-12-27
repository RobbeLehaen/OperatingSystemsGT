#include "datamgr.h"
#include "connmgr.h"
#include <inttypes.h>
#include "lib/dplist.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define RUN_AVG_LENGTH 5

static dplist_t *sensor_list = NULL;
static double min_temp = SET_MIN_TEMP;
static double max_temp = SET_MAX_TEMP;

int element_compare(void *x, void *y) {
    sensor_data_t *sensor_x = (sensor_data_t *)x;
    sensor_data_t *sensor_y = (sensor_data_t *)y;
    if (sensor_x->id < sensor_y->id) return -1;
    if (sensor_x->id > sensor_y->id) return 1;
    return 0;
}

void *copy_sensor_data(void *element) {
    if (element == NULL) return NULL;

    sensor_data_t *copy = malloc(sizeof(sensor_data_t));
    if (copy == NULL) {
        write_log("Memory allocation failed for sensor data copy");
        exit(EXIT_FAILURE);
    }

    *copy = *(sensor_data_t *)element;
    return copy;
}

void free_sensor_data(void **element) {
    if (element != NULL && *element != NULL) {
        free(*element);
        *element = NULL;
    }
}

void datamgr_init() {
    sensor_list = dpl_create(copy_sensor_data, free_sensor_data, element_compare);
    if (sensor_list == NULL) {
        write_log("Failed to create sensor list.\n");
        exit(1);
    }
}

void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data) {
    if (fp_sensor_map == NULL) {
        write_log("Sensor map file could not be opened\n");
        exit(EXIT_FAILURE);
    }

    uint16_t room_id, sensor_id;
    while (fscanf(fp_sensor_map, "%u %u\n", (unsigned int *)&room_id, (unsigned int *)&sensor_id) == 2) {
        sensor_data_t *sensor = malloc(sizeof(sensor_data_t));
        if (sensor == NULL) {
            write_log("Error: Memory allocation failed for sensor data\n");
            exit(EXIT_FAILURE);
        }
        sensor->room_id = room_id;
        sensor->id = sensor_id;
        sensor->value = 0;
        sensor->ts = 0;

        dpl_insert_at_index(sensor_list, sensor, dpl_size(sensor_list), true);
        free(sensor);
    }
}

void datamgr_free() {
    dpl_free(&sensor_list, true);
}

int datamgr_process_data(sensor_data_t *data) {
    int index = dpl_get_index_of_element(sensor_list, data);
    if (index == -1) {
        char log_msg[128];
        snprintf(log_msg, sizeof(log_msg), "Received sensor data with invalid sensor node ID %" PRIu16, data->id);
        write_log(log_msg);
        return DATAMGR_FAILURE;
    }

    sensor_data_t *sensor = (sensor_data_t *)dpl_get_element_at_index(sensor_list, index);
    sensor->value = data->value;
    sensor->ts = data->ts;

    double avg = datamgr_get_avg(sensor->id);
    fprintf(stdout, "Room %u: Sensor %u Running Avg = %.2f°C\n", sensor->room_id, sensor->id, avg);

    char log_msg[128];
    if (avg < min_temp) {
        snprintf(log_msg, sizeof(log_msg), "Sensor node %" PRIu16 " reports it’s too cold (avg temp = %.2f)", sensor->id, avg);
        write_log(log_msg);
    } else if (avg > max_temp) {
        snprintf(log_msg, sizeof(log_msg), "Sensor node %" PRIu16 " reports it’s too hot (avg temp = %.2f)", sensor->id, avg);
        write_log(log_msg);
    }

    return DATAMGR_SUCCESS;
}

sensor_value_t datamgr_get_avg(sensor_id_t sensor_id) {
    sensor_data_t temp_sensor;
    temp_sensor.id = sensor_id;

    int index = dpl_get_index_of_element(sensor_list, &temp_sensor);
    if (index == -1) return 0.0;

    int count = 0;
    double sum = 0.0;
    for (int i = index; i >= 0 && count < RUN_AVG_LENGTH; i--) {
        sensor_data_t *sensor = (sensor_data_t *)dpl_get_element_at_index(sensor_list, i);
        if (sensor->id == sensor_id) {
            sum += sensor->value;
            count++;
        }
    }

    return (count > 0) ? sum / count : 0.0;
}
