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
#include <sys/wait.h>

#define BUFFER_SIZE 1024

static int log_pipe[2];
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static pid_t log_process_pid = -1;

// Function to initialize logging
int init_logging() {
    // Create a pipe for communication
    if (pipe(log_pipe) == -1) {
        perror("Failed to create log pipe");
        return -1;
    }

    // Fork the logger process
    log_process_pid = fork();
    if (log_process_pid == -1) {
        perror("Failed to fork logger process");
        return -1;
    }

    if (log_process_pid == 0) {
        close(log_pipe[1]);

        // Open the log file
        FILE *log_file = fopen("gateway.log", "w");
        if (!log_file) {
            perror("Failed to open log file");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        char timestamp[128];
        int seq_num = 0;

        // Logger process main loop
        while (1) {
            ssize_t bytes_read = read(log_pipe[0], buffer, BUFFER_SIZE - 1);
            if (bytes_read <= 0) {
                if (bytes_read == 0) {
                    break;
                } else {
                    perror("Logger process read error");
                    break;
                }
            }

            buffer[bytes_read] = '\0';

            // Check for termination message
            if (strcmp(buffer, "EXIT") == 0) {
                break;
            }

            // Generate a timestamp
            time_t now = time(NULL);
            struct tm *tm_info = localtime(&now);
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

            // Process each line in the buffer
            char *line = strtok(buffer, "\n");
            while (line != NULL) {
                fprintf(log_file, "%d %s %s\n", seq_num++, timestamp, line);
                fflush(log_file);
                line = strtok(NULL, "\n");
            }
        }

        // Clean up logger resources
        fclose(log_file);
        close(log_pipe[0]);
        exit(EXIT_SUCCESS);
    } else {
        close(log_pipe[0]);
        return 0;
    }
}


// Function to write log messages
void write_log(const char *message) {
    pthread_mutex_lock(&log_mutex);

    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s\n", message);
    write(log_pipe[1], buffer, strlen(buffer));

    pthread_mutex_unlock(&log_mutex);
}


// Function to cleanup logging
void cleanup_logging() {
    if (log_process_pid > 0) {
        const char *termination_message = "EXIT";
        write(log_pipe[1], termination_message, strlen(termination_message));

        close(log_pipe[1]);

        int status;
        waitpid(log_process_pid, &status, 0);
    }
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
    write_log("Connection manager resources cleaned up.\n");
}


void *handle_client(void *arg) {
    tcpsock_t *client = (tcpsock_t *)arg;
    sensor_data_t data = {0};
    int bytes, result;
    int first_message = 1;
    char log_msg[256];

    do {
        // Receive Sensor ID
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *)&data.id, &bytes);
        if (result != TCP_NO_ERROR || bytes == 0) break;

        // Receive Sensor Value
        bytes = sizeof(data.value);
        result = tcp_receive(client, (void *)&data.value, &bytes);
        if (result != TCP_NO_ERROR || bytes == 0) break;

        // Receive Sensor Timestamp
        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void *)&data.ts, &bytes);
        if (result != TCP_NO_ERROR || bytes == 0) break;

        snprintf(log_msg, sizeof(log_msg),
                 "handle_client: Received data - Sensor ID = %" PRIu16 ", Value = %.2f, Timestamp = %ld",
                 data.id, data.value, (long int)data.ts);
        write_log(log_msg);

        if (first_message) {
            snprintf(log_msg, sizeof(log_msg), "Sensor node %" PRIu16 " has opened a new connection", data.id);
            write_log(log_msg);
            first_message = 0;
        }

        sensor_data_t local_data = {0};
        local_data.id = data.id;
        local_data.value = data.value;
        local_data.ts = data.ts;
        local_data.processed = 0;

        // Insert data into the shared buffer
        if (sbuffer_insert(shared_buffer, &local_data) != SBUFFER_SUCCESS) {
            write_log("handle_client: Failed to insert data into shared buffer");
            break;
        }
    } while (1);

    // Handle connection closure or error
    if (result == TCP_CONNECTION_CLOSED) {
        snprintf(log_msg, sizeof(log_msg), "Sensor node %" PRIu16 " has closed the connection", data.id);
        write_log(log_msg);
    } else {
        write_log("handle_client: Connection error occurred");
    }

    tcp_close(&client);

    pthread_mutex_lock(&state.conn_mutex);
    state.conn_counter--;
    if (state.conn_counter == 0) {
        state.server_running = 0;
    }
    pthread_mutex_unlock(&state.conn_mutex);

    write_log("handle_client: Client thread exiting");
    pthread_exit(NULL);
}

// Main server loop to listen and manage connections
void connmgr_listen() {
    pthread_t threads[state.max_connections];
    int thread_index = 0;

    printf("Connection manager listening...\n");

    while (1) {
        tcpsock_t *client;

        pthread_mutex_lock(&state.conn_mutex);
        if (!state.server_running) {
            pthread_mutex_unlock(&state.conn_mutex);
            break;
        }
        pthread_mutex_unlock(&state.conn_mutex);

        pthread_mutex_lock(&state.conn_mutex);
        if (state.conn_counter >= state.max_connections) {
            pthread_mutex_unlock(&state.conn_mutex);
            printf("Maximum client limit reached (%d). Stopping server.\n", state.max_connections);
            break;
        }
        pthread_mutex_unlock(&state.conn_mutex);

        if (tcp_wait_for_connection(state.server_socket, &client) != TCP_NO_ERROR) {
            fprintf(stderr, "Error accepting client connection\n");
            continue;
        }

        pthread_mutex_lock(&state.conn_mutex);
        state.conn_counter++;
        pthread_mutex_unlock(&state.conn_mutex);

        printf("Accepted new client connection (%d/%d)\n", state.conn_counter, state.max_connections);

        if (pthread_create(&threads[thread_index++], NULL, handle_client, client) != 0) {
            fprintf(stderr, "Failed to create thread for client.\n");
            tcp_close(&client);
            pthread_mutex_lock(&state.conn_mutex);
            state.conn_counter--;
            pthread_mutex_unlock(&state.conn_mutex);
            continue;
        }
    }

    printf("Waiting for all threads to complete...\n");
    for (int i = 0; i < thread_index; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All client threads have finished. Connection manager shutting down.\n");
}
