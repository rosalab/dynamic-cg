#!/bin/bash

# Benchmark parameters
SERVER="127.0.0.1"
PORT="11211"
DURATION="50"
THREADS="4"
CLIENTS="10"
RATIO="1:10"  # Ratio of write to read operations

# Start memcached in the background
memcached -u nobody -d -m 64 -p $PORT

# Wait a moment to ensure memcached is up
sleep 1

echo "Starting memcached benchmark using memtier_benchmark..."
echo "Server: $SERVER:$PORT"
echo "Duration: $DURATION seconds"
echo "Threads: $THREADS"
echo "Clients per thread: $CLIENTS"
echo "Write/Read Ratio: $RATIO"
echo

# Run memtier_benchmark
memtier_benchmark --server=$SERVER --port=$PORT \
    --protocol=memcache_text --threads=$THREADS --clients=$CLIENTS \
    --ratio=$RATIO --test-time=$DURATION

echo
echo "Benchmark completed."
