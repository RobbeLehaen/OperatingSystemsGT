make all
port=5678
clients=3

echo -e "Starting gateway under Valgrind"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./sensor_gateway $port $clients &
GATEWAY_PID=$!
sleep 3

echo -e "Starting 3 sensor nodes"
./sensor_node 15 1 127.0.0.1 $port &
sleep 2
./sensor_node 21 3 127.0.0.1 $port &
sleep 2
./sensor_node 37 2 127.0.0.1 $port &
sleep 11

echo -e "Stopping sensor nodes"
killall sensor_node
sleep 30

echo -e "Stopping gateway"
kill $GATEWAY_PID
