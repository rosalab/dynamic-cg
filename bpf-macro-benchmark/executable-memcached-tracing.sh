#!/bin/bash

# Benchmark parameters
SERVER="127.0.0.1"
PORT="11211"
DURATION="5"        # Duration in seconds
THREADS="4"
CLIENTS="10"
RATIO="1:10"         # Write/Read ratio

# Start memcached in the background and get its PID
echo "Starting memcached..."
# memcached -u nobody -d -m 64 -p $PORT &
trace-cmd record -p function -c -q -F memcached -u nobody -d -m 64 -p $PORT &
MEMCACHED_PID=$!

# Wait to ensure memcached is up
sleep 2
echo "Memcached started with PID: $MEMCACHED_PID"

# Run the benchmark
echo "Running memtier_benchmark..."
/memtier_benchmark/memtier_benchmark --server=$SERVER --port=$PORT \
    --protocol=memcache_text --threads=$THREADS --clients=$CLIENTS \
    --ratio=$RATIO --test-time=$DURATION

# Process the trace data
echo "Processing trace data..."
trace-cmd report > memcached_trace.txt

echo "Benchmark and tracing completed."
echo "Trace data saved to memcached_trace.txt"
