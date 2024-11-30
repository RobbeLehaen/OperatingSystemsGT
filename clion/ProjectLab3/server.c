#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <inttypes.h>
#include "server.h"

int server_init(server_state_t *state, int port, int max_connections) {
    if (tcp_passive_open(&state->server_socket, port) != TCP_NO_ERROR) {
        fprintf(stderr, "Failed to open server socket on port %d\n", port);
        return -1;
    }

    state->max_connections = max_connections;
    state->conn_counter = 0;
    pthread_mutex_init(&state->conn_mutex, NULL);

    printf("Server initialized on port %d with max connections %d\n", port, max_connections);
    return 0;
}

void server_cleanup(server_state_t *state) {
    tcp_close(&state->server_socket);
    pthread_mutex_destroy(&state->conn_mutex);
    printf("Server resources cleaned up\n");
}

void *handle_client(void *arg) {
    tcpsock_t *client = (tcpsock_t *)arg;
    sensor_data_t data;
    int bytes, result;

    printf("Handling new client connection\n");

    do {
        // Attempt to receive sensor data
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *)&data.id, &bytes);

        if (result == TCP_NO_ERROR && bytes > 0) {
            bytes = sizeof(data.value);
            result = tcp_receive(client, (void *)&data.value, &bytes);

            bytes = sizeof(data.ts);
            result = tcp_receive(client, (void *)&data.ts, &bytes);

            if (result == TCP_NO_ERROR && bytes > 0) {
                printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n",
                       data.id, data.value, (long int)data.ts);
            }
        }

        // Handle errors or client disconnection
        if (result != TCP_NO_ERROR || bytes == 0) {
            if (result == TCP_CONNECTION_CLOSED) {
                printf("Peer has closed the connection\n");
            } else {
                printf("Error occurred on connection to peer\n");
            }
            break;
        }
    } while (1);

    // Clean up the client socket and exit the thread
    tcp_close(&client);
    printf("Client thread exiting.\n");
    pthread_exit(NULL);
}


void server_run(server_state_t *state) {
    pthread_t threads[MAX_THREADS];
    int thread_index = 0;

    while (1) {
        tcpsock_t *client;

        // Check if maximum connections have been served
        pthread_mutex_lock(&state->conn_mutex);
        if (state->conn_counter >= state->max_connections) {
            pthread_mutex_unlock(&state->conn_mutex);
            printf("Maximum client limit reached (%d). Shutting down server.\n", state->max_connections);
            break; // Exit the server loop
        }
        pthread_mutex_unlock(&state->conn_mutex);

        // Accept a new connection
        if (tcp_wait_for_connection(state->server_socket, &client) != TCP_NO_ERROR) {
            fprintf(stderr, "Error accepting client connection\n");
            continue;
        }

        // Check connection limit immediately after accepting the connection
        pthread_mutex_lock(&state->conn_mutex);
        if (state->conn_counter >= state->max_connections) {
            pthread_mutex_unlock(&state->conn_mutex);
            printf("Connection rejected: Maximum client limit reached (%d).\n", state->max_connections);
            tcp_close(&client); // Immediately close the connection
            continue; // Skip further processing
        }
        state->conn_counter++;
        pthread_mutex_unlock(&state->conn_mutex);

        printf("Accepted client connection (%d/%d)\n", state->conn_counter, state->max_connections);

        // Create a thread for the new client
        if (pthread_create(&threads[thread_index++], NULL, handle_client, client) != 0) {
            fprintf(stderr, "Failed to create thread\n");
            tcp_close(&client);
            continue;
        }
    }

    // Wait for all threads to complete
    printf("Waiting for active threads to finish...\n");
    for (int i = 0; i < thread_index; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All client threads have finished. Server is shutting down.\n");
}