#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h> // Include string.h for strcspn function

#define FREQUENCY 5           // Frequency of readings in seconds
#define MIN_TEMPERATURE -10.0 // Minimum realistic outdoor temperature
#define MAX_TEMPERATURE 35.0  // Maximum realistic outdoor temperature

// Function to generate a random temperature
double generate_random_temperature() {
    return (rand() / (double)RAND_MAX) * (MAX_TEMPERATURE - MIN_TEMPERATURE) + MIN_TEMPERATURE;
}

int main() {
    // Seed the random number generator
    srand(time(NULL));

    // Infinite loop to continuously take temperature readings
    while (1) {
        // Generate a random temperature reading
        double temperature = generate_random_temperature();

        // Get the current date and time
        time_t now = time(NULL);
        char* datetime = ctime(&now); // Convert to local time string

        // Remove the newline character from datetime
        datetime[strcspn(datetime, "\n")] = 0;

        // Print the temperature and date/time
        printf("Temperature = %.2f @%s\n", temperature, datetime);

        // Wait for the specified frequency
        sleep(FREQUENCY);
    }

    return 0;
}

