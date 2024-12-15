#ifndef CONNMGR_H
#define CONNMGR_H

#include "sbuffer.h"

int init_logging();
void write_log(const char *message);
void cleanup_logging();

/**
 * Initializes the connection manager.
 * Opens a server socket on the specified port and sets up the connection state.
 *
 * @param port The port number to listen on.
 * @param max_clients The maximum number of simultaneous client connections.
 * @param buffer Pointer to the shared buffer for storing sensor data.
 * @return 0 on success, -1 on failure.
 */
int connmgr_init(int port, int max_clients, sbuffer_t *buffer);

/**
 * Starts the connection manager's main loop.
 * Listens for and accepts client connections, creating a thread for each client.
 * Stops accepting new connections when the maximum number of clients is reached.
 */
void connmgr_listen();

/**
 * Cleans up resources used by the connection manager.
 * Closes the server socket and frees associated resources.
 */
void connmgr_cleanup();

#endif // CONNMGR_H
