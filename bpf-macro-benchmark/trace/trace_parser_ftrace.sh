#!/bin/bash

# Check if a file is provided
if [ -z "$1" ]; then
  echo "Usage: $0 <input_file>"
  exit 1
fi

input_file="$1"

# Regex patterns to match function entries and durations
entry_pattern="^[[:space:]]*[0-9]+\)[[:space:]]+\|[[:space:]]+([a-zA-Z0-9_.]+)\(\)[[:space:]]*\{$"
duration_pattern="^[[:space:]]*[0-9]+\)[[:space:]]+[0-9.]+ us[[:space:]]+\|[[:space:]]+([a-zA-Z0-9_.]+)\(\)"

# Temporary file to store results
temp_file=$(mktemp)

# Parse the file to extract function names and durations
while IFS= read -r line; do
  # Match function entry
  if [[ $line =~ $entry_pattern ]]; then
    func_name="${BASH_REMATCH[1]}"
    echo "$func_name" >> "$temp_file"
  fi

  # Match function duration
  if [[ $line =~ $duration_pattern ]]; then
    func_name="${BASH_REMATCH[1]}"
    echo "$func_name" >> "$temp_file"
  fi
done < "$input_file"

# Count occurrences of each function
sort "$temp_file" | uniq -c | sort -nr

# Clean up temporary file
rm -f "$temp_file"
