#!/usr/bin/env bash

# This script is supposed to 
# @author Egor Lukiyanov (egor@vt.edu)

# example usage:
# ./benchmark.sh 

# Check if the test argument is provided
if [ "$#" -ne 4 ]; then
    echo "Usage: bpf_program_script output_directory times_to_run_tests executable(and its options)"
    exit 1
fi

# compiles *.bpf.c and link.c with provided makefile
make -C "./bpf-programs"

# ---------
# Arguments
# ---------

# set this variable to a script that attaches any bpf program you need
# make sure the programs are pinned to the directory /sys/fs/bpf
# example script included in directory, called link.c
bpfprogscript=$1

# directory that has the outputted results
output=$2

# times to run tests
iterations=$3

# set this variable to the executable you want to test
# NOTE: executable should output its results to stdout
# shifts over the arguments so that all the executable options can be included
shift 3
executable_and_options="$@"

# --------

# Run the benchmark in order to get its most frequently used tracepoints
# cant be ran as the baseline as ftrace impacts performance
#  > shouldnt I run ftrace as the baseline to compensate for the performance overhead?

# Scheduler tracepoints stat_sleep, stat_iowait, stat_b
# locked and stat_runtime require the kernel parameter schedstats=enable or kernel.sched_schedstats=1
echo "running benchmark for tracing output"

# note enabling stats decreases performance!!!
# used for bpftool stat execution counter?
# sysctl -w kernel.bpf_stats_enabled=1

# Runs the preliminary benchmark to get a baseline performance metric
echo "Starting baseline benchmark..."

mkdir -p "$output"

# Detach any bpf programs for baseline
rm -r /sys/fs/bpf/*

for (( counter=1; counter<=iterations; counter++ ))
do

    # RUN BASELINE
    baseline=$(eval "$executable_and_options")
    # RUN BASELINE

     echo "$baseline" > "$output/baseline_$counter.txt"

    # PROCESS RESULTS
    # echo "$baseline" > "$output/baseline.txt"

done

# Attach the ebpf program(s) using the provided script
eval "$bpfprogscript"

for (( counter=1; counter<=iterations; counter++ ))
do
    # RUN BENCHMARK WITH ATTACHED EBPF
    command_output=$(eval "$executable_and_options")

    # Create a new file in the output directory with the baseline and iteration count
    echo "$command_output" > "$output/result_$counter.txt"

    echo "Iteration $counter: Output written to $output"

done

# cleanup bpf programs
rm -r /sys/fs/bpf/*

echo "Finished executing files at: $output"

