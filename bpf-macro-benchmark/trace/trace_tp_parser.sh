#!/bin/bash

# Check if a file is provided
if [ -z "$1" ]; then
  echo "Usage: $0 <input_file>"
  exit 1
fi

input_file="$1"

cat $input_file | grep : | awk '{print $4}' | sort | uniq -c | sort -nr
