import os
import subprocess
import time
import argparse

# Paths (adjust if necessary)
KERN_FUNC_FILE = "kern_func.txt"  # This will read kernel functions from the file
OUTPUT_FILE = "generic.ftrace.kern.c"
OBJ_FILE = "generic.ftrace.kern.o"
MAKE_CMD = "make"
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
    Reads a newline-separated file and puts all elements into a list.
    """
    with open(filepath, 'r') as file:
        return [line.strip() for line in file.readlines() if line.strip()]

def generate_c_file(func, output_filepath):
    """
    Generates the C file using the template and the kernel function.
    """
    with open(output_filepath, 'w') as f:
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

def run_link_user(functions, xdp_program):
    """
    Calls the link.user executable for each function with the correct parameters.
    Also, attaches the XDP program and runs it from the ../ directory.
    """
    for func in functions:
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

        print(f"RUNNING XDP TEST: {xdp_program} in directory {xdp_dir}")
        time.sleep(2)

        try:
            os.chdir(xdp_dir)  # Change directory to ../
            subprocess.run([f"./{xdp_program}"], check=True)
            print(f"Ran and attached {xdp_program} successfully.")
        except subprocess.CalledProcessError as e:
            print(f"Error running {xdp_program}: {e}")
        finally:
            os.chdir("./bpf-programs")

        time.sleep(2)
        print(f"STOPPED RUNNING XDP TEST: {xdp_program}")

        print("DETACHING BPF PROGRAMS")
        detatch_bpf(func)

def detatch_bpf(pinned):
    """
    Detaches the BPF program.
    """
    detatch_cmd = ["rm", "-r", f"/sys/fs/bpf/fentry_special_{pinned}"]

    try:
        subprocess.run(detatch_cmd, check=True)
        print(f"Detached bpf program correctly")
    except subprocess.CalledProcessError as e:
        print(f"Could not detatch")

def main():
    # Parse command-line arguments
    parser = argparse.ArgumentParser(description='Attach XDP program and kernel function.')
    parser.add_argument('xdp_prog', type=str, help='XDP program to run')

    args = parser.parse_args()

    # Step 1: Read kernel functions from file
    functions = read_file(KERN_FUNC_FILE)

    # Step 4: Run the link.user executable to attach the eBPF program
    run_link_user(functions, args.xdp_prog)

if __name__ == "__main__":
    main()
