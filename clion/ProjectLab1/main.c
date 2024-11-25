#include <stdlib.h>
#include <stdio.h>
#include "datamgr.h"
#include <time.h>

int main() {
    FILE *map = fopen("room_sensor.map", "r");
    FILE *data = fopen("sensor_data", "rb");

    // Check if the files exist and are opened properly
    if (map == NULL) {
        fprintf(stderr, "Error: Could not open room_sensor.map\n");
        return EXIT_FAILURE;
    }
    if (data == NULL) {
        fprintf(stderr, "Error: Could not open sensor_data\n");
        fclose(map);  // Close the already opened file
        return EXIT_FAILURE;
    }

    datamgr_init();

    // Call the data manager function
    datamgr_parse_sensor_files(map, data);

    // Free the resources after usage
    datamgr_free();

    fclose(map);
    fclose(data);

    return EXIT_SUCCESS;
}
