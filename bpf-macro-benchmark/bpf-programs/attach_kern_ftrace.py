import os
import random
import subprocess

# Paths (adjust if necessary)
KERN_FUNC_FILE = "kern_func.txt"
OUTPUT_FILE = "generic.ftrace.kern.c"
OBJ_FILE = "generic.ftrace.kern.o"
MAKE_CMD = "make"
LINK_USER_EXEC = "./link.user"
SYS_MAP_PATH = '/lib/modules/6.11.0-rc5/build/System.map'
SEED = 6722

# Template for the C file
C_TEMPLATE = """
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

char LICENSE[] SEC("license") = "Dual BSD/GPL";

SEC("fentry/{function}")
int {function}(void *ctx)
{{
    return 0;
}}
"""

def read_kern_func_file(filepath):
    """
    Reads the kern_func.txt file and returns the list of kernel functions.
    """
    with open(filepath, 'r') as file:
        functions = [line.strip() for line in file.readlines() if line.strip()]
    # print(functions)
    return functions

def generate_c_file(functions, output_filepath):
    """
    Generates the C file using the template and the list of functions.
    """
    with open(output_filepath, 'w') as f:
        # Generate one hookpoint for each function in kern_func.txt
        # for func in functions:
        func = functions[0]
        f.write(C_TEMPLATE.format(function=func))
    print(f"Generated {output_filepath}")

def run_make():
    """
    Runs the make command to compile the BPF program.
    """
    try:
        subprocess.run(MAKE_CMD, check=True)
        print("Make command executed successfully.")
    except subprocess.CalledProcessError as e:
        print(f"Error occurred during make: {e}")

def run_link_user(functions):
    """
    Calls the link.user executable for each function with the correct parameters.
    """
    # for func in functions:
    func = functions[0]

    attach_cmd = [LINK_USER_EXEC, f"fentry/{func}", OBJ_FILE, func]
    try:
        subprocess.run(attach_cmd, check=True)
        print(f"Attached function {func} successfully.")
    except subprocess.CalledProcessError as e:
        print(f"Error attaching function {func}: {e}")

def read_system_map(filepath):
    """
    Reads the System.map file and extracts lines containing function addresses.
    """
    with open(filepath, 'r') as file:
        lines = file.readlines()

    # Filter lines containing 'T' which indicates a global function (in the text segment)
    functions = [line.split()[-1] for line in lines if ' T ' in line]
    return functions

def select_random_functions(functions, seed, count=30):
    """
    Selects `count` random functions from the list using a user-defined seed.
    """
    random.seed(seed)  # Set the seed for reproducibility
    return random.sample(functions, count)

def main():

    functions = read_kern_func_file(KERN_FUNC_FILE)

    # Step 2: Generate the C file
    generate_c_file(functions, OUTPUT_FILE)

    # Step 3: Run the make command
    run_make()

    # Step 4: Run the link.user executable to attach the eBPF program
    run_link_user(functions)

if __name__ == "__main__":
    main()
