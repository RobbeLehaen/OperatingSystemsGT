#include <stdio.h>
#include <stdlib.h>
#include "server.h"
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <port> <max_connections>\n", argv[0]);
        return -1;
    }

    int port = atoi(argv[1]);
    int max_connections = atoi(argv[2]);

    server_state_t server;
    if (server_init(&server, port, max_connections) != 0) {
        fprintf(stderr, "Failed to initialize server\n");
        return -1;
    }

    printf("Multi-threaded server is starting\n");
    server_run(&server);

    server_cleanup(&server);
    printf("Server is shutting down\n");
    return 0;
}

