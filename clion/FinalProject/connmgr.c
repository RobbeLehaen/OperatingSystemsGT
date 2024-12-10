#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <inttypes.h>
#include "connmgr.h"
#include "lib/tcpsock.h"
#include "sbuffer.h"

// Server state structure
typedef struct connmgr_state {
    tcpsock_t *server_socket;
    int max_connections;
    int conn_counter;
    pthread_mutex_t conn_mutex;
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
    pthread_mutex_init(&state.conn_mutex, NULL);
    shared_buffer = buffer;

    printf("Server initialized on port %d with max clients %d\n", port, max_clients);
    return 0;
}

// Cleanup server resources
void connmgr_cleanup() {
    tcp_close(&state.server_socket);
    pthread_mutex_destroy(&state.conn_mutex);
    printf("Connection manager resources cleaned up.\n");
}

// Handle a single client connection
void *handle_client(void *arg) {
    tcpsock_t *client = (tcpsock_t *)arg;
    sensor_data_t data;
    int bytes, result;

    printf("Handling new client connection\n");

    do {
        // Receive sensor ID
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *)&data.id, &bytes);
        if (result != TCP_NO_ERROR || bytes == 0) break;

        // Receive sensor value
        bytes = sizeof(data.value);
        result = tcp_receive(client, (void *)&data.value, &bytes);
        if (result != TCP_NO_ERROR || bytes == 0) break;

        // Receive sensor timestamp
        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void *)&data.ts, &bytes);
        if (result != TCP_NO_ERROR || bytes == 0) break;

        printf("Received data: Sensor ID = %" PRIu16 ", Value = %.2f, Timestamp = %ld\n",
               data.id, data.value, (long int)data.ts);

        // Insert data into the shared buffer
        if (sbuffer_insert(shared_buffer, &data) != SBUFFER_SUCCESS) {
            fprintf(stderr, "Failed to insert data into shared buffer\n");
            break;
        }
    } while (1);

    // Handle disconnection
    if (result == TCP_CONNECTION_CLOSED) {
        printf("Client disconnected.\n");
    } else {
        fprintf(stderr, "Error occurred on connection.\n");
    }

    // Close the client socket and decrement connection counter
    tcp_close(&client);

    pthread_mutex_lock(&state.conn_mutex);
    state.conn_counter--;
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
