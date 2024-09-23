#!/bin/bash

# Benchmark parameters
SERVER="127.0.0.1"
PORT="11211"
DURATION="30"        # Duration in seconds
THREADS="4"
CLIENTS="10"
RATIO="1:10"         # Write/Read ratio

FTRACE_PATH="/sys/kernel/debug/tracing"
TRACE_FILE="${FTRACE_PATH}/trace"

echo "Starting memcached..."
memcached -u nobody -d -m 64 -p $PORT
MEMCACHED_PID=$(pgrep memcached | head -n 1)

# Verify memcached is running and bound to the port
if lsof -i :$PORT > /dev/null; then
    echo "memcached is running and bound to port $PORT"
else
    echo "memcached failed to bind to port $PORT"
    exit 1
fi

echo "Setting up ftrace..."
echo function_graph > ${FTRACE_PATH}/current_tracer
echo $MEMCACHED_PID > ${FTRACE_PATH}/set_ftrace_pid
echo 1 > ${FTRACE_PATH}/tracing_on

# Clear previous trace logs
echo > ${TRACE_FILE}

# Run the benchmark
echo "Running memtier_benchmark..."
memtier_benchmark --server=$SERVER --port=$PORT \
    --protocol=memcache_text --threads=$THREADS --clients=$CLIENTS \
    --ratio=$RATIO --test-time=$DURATION

# Stop ftrace
echo 0 > ${FTRACE_PATH}/tracing_on

echo "Saving trace data..."
cat ${TRACE_FILE} > /linux/memcached_ftrace.txt

# Clean up ftrace
echo > ${FTRACE_PATH}/trace
echo > ${FTRACE_PATH}/set_ftrace_pid
echo "ftrace stopped and trace saved to /linux/memcached_ftrace.txt."

kill $MEMCACHED_PID
echo "memcached stopped."