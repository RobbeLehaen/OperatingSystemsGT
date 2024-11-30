# Targets to build both server and sensor node
all: server sensor_node

# Clean up generated files
clean:
	rm -f server sensor_node

# Build the test server with main.c, server.c, and lib/tcpsock.c
server: main.c server.c lib/tcpsock.c server.h
	gcc -g main.c server.c lib/tcpsock.c -o server -lpthread

# Build the sensor node
sensor_node: sensor_node.c lib/tcpsock.c lib/tcpsock.h config.h
	gcc -g sensor_node.c lib/tcpsock.c -o sensor_node

# Run the server
runserver: server
	./server 5678 4

# Run the first client
runclient1: sensor_node
	./sensor_node 1 2 127.0.0.1 5678

# Run the second client
runclient2: sensor_node
	./sensor_node 2 5 127.0.0.1 5678

# Run the second client
runclient3: sensor_node
	./sensor_node 3 2 127.0.0.1 5678

# Run the second client
runclient4: sensor_node
	./sensor_node 4 2 127.0.0.1 5678

# Run the second client
runclient5: sensor_node
	./sensor_node 5 1 127.0.0.1 5678

