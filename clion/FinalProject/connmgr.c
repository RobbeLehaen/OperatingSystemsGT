#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <inttypes.h>
#include "connmgr.h"
#include "lib/tcpsock.h"
#include "sbuffer.h"
#include <unistd.h>
#include <time.h>

#define LOG_BUFFER_SIZE 1024

// Global variables for logging
static int log_pipe[2];
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t log_thread;

// Logging thread function
void *log_thread_func(void *arg) {
    FILE *log_file = fopen("gateway.log", "w");
    if (!log_file) {
        perror("Failed to open gateway.log");
        pthread_exit(NULL);
    }

    char buffer[LOG_BUFFER_SIZE];
    char timestamp[128];
    int seq_num = 0;

    while (1) {
        ssize_t bytes_read = read(log_pipe[0], buffer, LOG_BUFFER_SIZE - 1);
        if (bytes_read <= 0) break; // Pipe closed or error occurred

        buffer[bytes_read] = '\0'; // Null-terminate the buffer

        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

        fprintf(log_file, "%d %s %s\n", seq_num++, timestamp, buffer);
        fflush(log_file);
    }

    fclose(log_file);
    pthread_exit(NULL);
}

// Function to initialize logging
int init_logging() {
    if (pipe(log_pipe) == -1) {
        perror("Failed to create log pipe");
        return -1;
    }

    if (pthread_create(&log_thread, NULL, log_thread_func, NULL) != 0) {
        perror("Failed to create log thread");
        return -1;
    }

    return 0;
}

// Function to write log messages
void write_log(const char *message) {
    pthread_mutex_lock(&log_mutex);

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%s\n", message);
    write(log_pipe[1], buffer, strlen(buffer));

    pthread_mutex_unlock(&log_mutex);
}


// Function to cleanup logging
void cleanup_logging() {
    close(log_pipe[1]); // Close write end of the pipe
    pthread_join(log_thread, NULL);
    close(log_pipe[0]); // Close read end of the pipe
}

// Server state structure
typedef struct connmgr_state {
    tcpsock_t *server_socket;
    int max_connections;
    int conn_counter;
    pthread_mutex_t conn_mutex;
    int server_running;
} connmgr_state_t;

static connmgr_state_t state;
static sbuffer_t *shared_buffer;

// Server initialization function
int connmgr_init(int port, int max_clients, sbuffer_t *buffer) {
    if (tcp_passive_open(&state.server_socket, port) != TCP_NO_ERROR) {
        fprintf(stderr, "Failed to open server socket on port %d\n", port);
        return -1;
    }

    state.max_connections = max_clients;
    state.conn_counter = 0;
    state.server_running = 1;
    pthread_mutex_init(&state.conn_mutex, NULL);
    shared_buffer = buffer;

    printf("Server initialized on port %d with max clients %d\n", port, max_clients);
    return 0;
}

// Cleanup server resources
void connmgr_cleanup() {
    state.server_running = 0;
    tcp_close(&state.server_socket);
    pthread_mutex_destroy(&state.conn_mutex);
    printf("Connection manager resources cleaned up.\n");
}


// Handle a single client connection
void *handle_client(void *arg) {
    tcpsock_t *client = (tcpsock_t *)arg;
    sensor_data_t data = {0};
    int bytes, result;

    // Log the opening of the connection
    char log_msg[128];
    snprintf(log_msg, sizeof(log_msg), "Sensor node %" PRIu16 " has opened a new connection", data.id);
    write_log(log_msg);

    printf("Handling new client connection\n");

    do {
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *)&data.id, &bytes);
        if (result != TCP_NO_ERROR || bytes == 0) break;

        bytes = sizeof(data.value);
        result = tcp_receive(client, (void *)&data.value, &bytes);
        if (result != TCP_NO_ERROR || bytes == 0) break;

        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void *)&data.ts, &bytes);
        if (result != TCP_NO_ERROR || bytes == 0) break;

        printf("Received data: Sensor ID = %" PRIu16 ", Value = %.2f, Timestamp = %ld\n",
               data.id, data.value, (long int)data.ts);

        if (sbuffer_insert(shared_buffer, &data) != SBUFFER_SUCCESS) {
            fprintf(stderr, "Failed to insert data into shared buffer\n");
            break;
        }
    } while (1);

    if (result == TCP_CONNECTION_CLOSED) {
        snprintf(log_msg, sizeof(log_msg), "Sensor node %" PRIu16 " has closed the connection", data.id);
        write_log(log_msg);
    } else {
        fprintf(stderr, "Error occurred on connection.\n");
    }

    tcp_close(&client);

    pthread_mutex_lock(&state.conn_mutex);
    state.conn_counter--;
    if (state.conn_counter == 0) {
        state.server_running = 0; // Signal server shutdown
    }
    pthread_mutex_unlock(&state.conn_mutex);

    printf("Client thread exiting.\n");
    pthread_exit(NULL);
}

// Main server loop to listen and manage connections
void connmgr_listen() {
    pthread_t threads[state.max_connections];
    int thread_index = 0;

    printf("Connection manager listening...\n");

    while (1) {
        tcpsock_t *client;

        // Check if server should stop running
        pthread_mutex_lock(&state.conn_mutex);
        if (!state.server_running) {
            pthread_mutex_unlock(&state.conn_mutex);
            break;
        }
        pthread_mutex_unlock(&state.conn_mutex);

        // Check if maximum connections have been reached
        pthread_mutex_lock(&state.conn_mutex);
        if (state.conn_counter >= state.max_connections) {
            pthread_mutex_unlock(&state.conn_mutex);
            printf("Maximum client limit reached (%d). Stopping server.\n", state.max_connections);
            break;
        }
        pthread_mutex_unlock(&state.conn_mutex);

        // Wait for a new connection
        if (tcp_wait_for_connection(state.server_socket, &client) != TCP_NO_ERROR) {
            fprintf(stderr, "Error accepting client connection\n");
            continue;
        }

        // Increment connection counter
        pthread_mutex_lock(&state.conn_mutex);
        state.conn_counter++;
        pthread_mutex_unlock(&state.conn_mutex);

        printf("Accepted new client connection (%d/%d)\n", state.conn_counter, state.max_connections);

        // Create a thread to handle the new client
        if (pthread_create(&threads[thread_index++], NULL, handle_client, client) != 0) {
            fprintf(stderr, "Failed to create thread for client.\n");
            tcp_close(&client);
            pthread_mutex_lock(&state.conn_mutex);
            state.conn_counter--;
            pthread_mutex_unlock(&state.conn_mutex);
            continue;
        }
    }

    // Wait for all threads to finish
    printf("Waiting for all threads to complete...\n");
    for (int i = 0; i < thread_index; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All client threads have finished. Connection manager shutting down.\n");
}
