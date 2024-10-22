import os
import random
import subprocess
import time

# Paths (adjust if necessary)
KERN_FUNC_FILE = "kern_func.txt"
XDP_TESTS_FILE = "xdp_progs.txt"
OUTPUT_FILE = "generic.ftrace.kern.c"
OBJ_FILE = "generic.ftrace.kern.o"
MAKE_CMD = "make"
SYS_MAP_PATH = '/lib/modules/6.11.0-rc5/build/System.map'
SEED = 6721

# Template for the C file
C_TEMPLATE = """
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

char LICENSE[] SEC("license") = "Dual BSD/GPL";

SEC("fentry/{function}")
int special_{function}(void *ctx)
{{
    bpf_printk("Testing {function}!!!\\n");
    return 0;
}}
"""

def read_file(filepath):
    """
    Reads a newline seperated file and puts all elements into a list
    """
    with open(filepath, 'r') as file:
        functions = [line.strip() for line in file.readlines() if line.strip()]
    # print(functions)
    return functions

def generate_c_file(func, output_filepath):
    """
    Generates the C file using the template and the list of functions.
    """
    with open(output_filepath, 'w') as f:
        # Generate one hookpoint for each function in kern_func.txt
        # for func in functions:
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

def run_link_user(functions, xdp_programs):
    """
    Calls the link.user executable for each function with the correct parameters.
    Also, attaches the XDP program and runs it from the ../ directory.
    """
    for xdp in xdp_programs:
        for func in functions:
            # func = functions[0]
            # xdp = xdp_programs[0]

            # Step 2: Generate the C file
            generate_c_file(func, OUTPUT_FILE)

            # Step 3: Run the make command
            run_make()

            attach_cmd = ["./link.user", f"fentry/{func}", OBJ_FILE, f"special_{func}"]

            # Always run XDP program from the ../ directory
            xdp_dir = "../"

            # Attach the BPF program
            try:
                subprocess.run(attach_cmd, check=True)
                print(f"Attached function {func} successfully.")
            except subprocess.CalledProcessError as e:
                print(f"Error attaching function {func}: {e}")

            print(f"RUNNING XDP TEST: {xdp} in directory {xdp_dir}")
            time.sleep(2)

            try:
                os.chdir(xdp_dir)  # Change directory to ../
                subprocess.run([f"./{xdp}"], check=True)
                print(f"Ran and attached {xdp} successfully.")
            except subprocess.CalledProcessError as e:
                print(f"Error running {xdp}: {e}")
            finally:
                os.chdir("./bpf-programs")

            time.sleep(2)
            print(f"STOPPED RUNNING XDP TEST: {xdp}")

            print("DETACHING BPF PROGRAMS")
            detatch_bpf(func)

def detatch_bpf(pinned):

    detatch_cmd = ["rm", "-r", f"/sys/fs/bpf/fentry_special_{pinned}"]

    try:
        subprocess.run(detatch_cmd, check=True)
        print(f"Detached bpf program correctly")
    except subprocess.CalledProcessError as e:
        print(f"Could not detatch")

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

    # Step 1: read files
    functions = read_file(KERN_FUNC_FILE)
    xdp_programs = read_file(XDP_TESTS_FILE)

    # Step 4: Run the link.user executable to attach the eBPF program
    run_link_user(functions, xdp_programs)

if __name__ == "__main__":
    main()
