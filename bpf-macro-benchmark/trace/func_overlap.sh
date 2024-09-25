#!/bin/bash

# bash script expects two txt files with a specific format outputted by another script called trace_parser
# example format looks like so:
# 19761 __rcu_read_unlock
# 19759 __rcu_read_lock
# 10251 _raw_spin_unlock_irqrestore
#  9984 __cond_resched
#  9862 _raw_spin_lock_irqsave
#  9288 fput
#  7733 _raw_spin_unlock
#  7588 _raw_spin_lock

# Check if two files are provided
if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <file1> <file2>"
  exit 1
fi

file1="$1"
file2="$2"

# Create associative arrays for storing function frequencies from both files
declare -A functions1
declare -A functions2

# Read file1 and store function names and their counts
total_count1=0
while IFS=' ' read -r count func_name; do
  # Trim leading/trailing spaces from func_name
  func_name="${func_name// }"
  if [[ -n "$count" && -n "$func_name" ]]; then
    functions1["$func_name"]=$count
    total_count1=$((total_count1 + count))  # Accumulate total function count for file1
  fi
done < "$file1"

# Read file2 and store function names and their counts
total_count2=0
while IFS=' ' read -r count func_name; do
  # Trim leading/trailing spaces from func_name
  func_name="${func_name// }"
  if [[ -n "$count" && -n "$func_name" ]]; then
    functions2["$func_name"]=$count
    total_count2=$((total_count2 + count))  # Accumulate total function count for file2
  fi
done < "$file2"

shared_functions=()
total_shared_percentage=0

for func_name in "${!functions1[@]}"; do
  if [[ -n "${functions2["$func_name"]}" ]]; then
    shared_functions+=("$func_name")

    norm_count1=$(echo "scale=7; (${functions1["$func_name"]} / $total_count1) * 100" | bc)
    norm_count2=$(echo "scale=7; (${functions2["$func_name"]} / $total_count2) * 100" | bc)
    
    min_norm_count=$(echo "$norm_count1 $norm_count2" | awk '{print ($1 < $2) ? $1 : $2}')

    total_shared_percentage=$(echo "scale=5; $total_shared_percentage + $min_norm_count" | bc)
  fi
done

# Sort shared functions by the overlap in frequencies (biggest overlap)
for func_name in "${shared_functions[@]}"; do
  norm_count1=$(echo "scale=7; (${functions1["$func_name"]} / $total_count1) * 100" | bc)
  norm_count2=$(echo "scale=7; (${functions2["$func_name"]} / $total_count2) * 100" | bc)
  
  min_norm_count=$(echo "$norm_count1 $norm_count2" | awk '{print ($1 < $2) ? $1 : $2}')
  
  echo "$min_norm_count% overlap in $func_name"

done | sort -n -r

total_functions1=${#functions1[@]}
total_functions2=${#functions2[@]}
shared_function_count=${#shared_functions[@]}

total_unique_functions=$(echo "${!functions1[@]} ${!functions2[@]}" | tr ' ' '\n' | sort | uniq | wc -l)

percent_name_overlap=$(echo "scale=2; $shared_function_count / $total_unique_functions * 100" | bc)

echo -e "\nNumber of matching functions: $shared_function_count, Total unique functions across both: $total_unique_functions"
# This is the amount of matching functions across both applications divided by the unique functions across both application.
echo "Percentage of shared function names across both applications:: $percent_name_overlap%"
# The same thing but weighted by the number of times the function appears in the application stack traced normalized to the total number of functions traced
echo "Average percentage of count overlap (normalized): $total_shared_percentage%"
