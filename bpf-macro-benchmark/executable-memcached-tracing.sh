#!/bin/bash

# Benchmark parameters
SERVER="127.0.0.1"
PORT="11211"
DURATION="1"        # Duration in seconds
THREADS="4"
CLIENTS="10"
RATIO="1:10"         # Write/Read ratio

# Start memcached in the background and get its PID
echo "Starting memcached..."
memcached -u nobody -d -m 64 -p $PORT
# memcached -u nobody -d -m 64 -p 11211

# trace-cmd record -p function -q -c -F memcached -u nobody -d -m 64 -p 11211

# trace-cmd record -p function -q -c -F memcached -u nobody -d -p $PORT

# MEMCACHED_PID=$!

# Wait to ensure memcached is up
# sleep 5

# Start tracing using trace-cmd for the specific memcached PID
# echo "Starting trace-cmd on memcached PID $MEMCACHED_PID..."
# trace-cmd record -p function -q -c -P $MEMCACHED_PID &
# 211
# trace-cmd record -p function -q -c -P 211

trace-cmd reset

trace-cmd start -p function_graph -b 10000

# Run the benchmark
echo "Running memtier_benchmark..."
/memtier_benchmark/memtier_benchmark --server=$SERVER --port=$PORT \
    --protocol=memcache_text --threads=$THREADS --clients=$CLIENTS \
    --ratio=$RATIO --test-time=$DURATION

trace-cmd stop

trace-cmd extract

# fg 

# Process the trace data
# echo "Processing trace data..."
trace-cmd report > /linux/memcached_trace2.txt

# echo "Benchmark and tracing completed."
# echo "Trace data saved to memcached_trace2.txt"
