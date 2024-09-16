#!/usr/bin/env bash

start_time=$(date +%s%N) # Capture start time in nanoseconds

# Execute the command
eval "./bpf-programs/trigger.user"

end_time=$(date +%s%N) # Capture end time in nanoseconds
execution_time=$((end_time - start_time)) # Calculate execution time in nanoseconds

# Echo the execution time in nanoseconds
echo "Execution time: $execution_time nanoseconds"