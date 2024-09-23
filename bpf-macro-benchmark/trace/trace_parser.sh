#!/bin/bash

# Check if a file is provided
if [ -z "$1" ]; then
  echo "Usage: $0 <input_file>"
  exit 1
fi

input_file="$1"

# Extract lines containing 'funcgraph_entry', extract function names, and count occurrences
grep "funcgraph_entry" "$input_file" | \
  sed -n 's/.*| *\([^ ]*\)().*/\1/p' | \
  sort | uniq -c | sort -nr
