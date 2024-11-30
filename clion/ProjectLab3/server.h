#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include "lib/tcpsock.h"
#include "config.h"

#define MAX_THREADS 100

typedef struct {
    tcpsock_t *server_socket;         // Server socket
    int max_connections;             // Maximum allowed connections
    int conn_counter;                // Connection counter
    pthread_mutex_t conn_mutex;      // Mutex for thread-safe connection counting
} server_state_t;

int server_init(server_state_t *state, int port, int max_connections);
void server_cleanup(server_state_t *state);
void *handle_client(void *arg);
void server_run(server_state_t *state);

#endif // SERVER_H
