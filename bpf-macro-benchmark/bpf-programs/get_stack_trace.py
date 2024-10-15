import subprocess

# Path to the output file where the stack traces will be saved
output_file = "stack_traces.txt"

def get_first_map_id():
    # Run the bpftool command to show all maps
    cmd = ["bpftool", "map", "show"]
    
    try:
        # Execute the bpftool command and capture the output
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        
        # Parse the output to extract the first map ID (assumes first line contains map ID)
        for line in result.stdout.splitlines():
            if "id" in line:
                # Extract the map ID (assumes format 'id <number>')
                map_id = line.split("id")[1].split()[0]
                return int(map_id)
        
        raise ValueError("No map ID found in bpftool output.")
    
    except subprocess.CalledProcessError as e:
        print(f"Error executing bpftool map show: {e}")
        print(f"Command output: {e.output}")
        raise

def extract_stack_traces(stack_trace_map_id):
    # Run the bpftool command to dump the contents of the stack trace map
    cmd = ["bpftool", "map", "dump", "id", str(stack_trace_map_id)]
    
    try:
        # Execute the bpftool command and capture the output
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        
        # Save the output to a file
        with open(output_file, "w") as f:
            f.write(result.stdout)
        
        print(f"Stack traces successfully written to {output_file}")
    
    except subprocess.CalledProcessError as e:
        print(f"Error executing bpftool: {e}")
        print(f"Command output: {e.output}")

if __name__ == "__main__":
    try:
        # Get the first map ID automatically
        map_id = get_first_map_id()
        print(f"Using map ID: {map_id}")
        
        # Extract and save the stack traces
        extract_stack_traces(map_id)
    
    except Exception as e:
        print(f"Failed to extract stack traces: {e}")
