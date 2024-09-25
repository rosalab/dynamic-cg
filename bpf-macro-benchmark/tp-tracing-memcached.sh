#!/bin/bash

# Benchmark parameters
SERVER="127.0.0.1"
PORT="11211"
DURATION="1"        # Duration in seconds
THREADS="1"
CLIENTS="1"
RATIO="1:1"         # Write/Read ratio

# Start memcached in the background and get its PID
echo "Starting memcached..."
sysctl -w kernel.bpf_stats_enabled=1
trace-cmd record -e all -b 100000 -c -F memcached -u nobody -m 64 $PORT &

# Wait to ensure memcached is up
sleep 5

MEMCACHED_PID=$(pgrep memcached | head -n 1)

echo "Memcached PID: $MEMCACHED_PID"
# ps -axjf

# Run the benchmark
echo "Running memtier_benchmark..."
memtier_benchmark --server=$SERVER --port=$PORT \
    --protocol=memcache_text --threads=$THREADS --clients=$CLIENTS \
    --ratio=$RATIO --test-time=$DURATION

# Process the trace data
kill $MEMCACHED_PID

sleep 5

trace-cmd report > /linux/memcached_trace_tp.txt

echo "Trace data saved to /linux/memcached_trace_tp.txt"
