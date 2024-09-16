/**
 * eBPF Loader and Attacher with Support for Multiple Hook Types
 * -------------------------------------------------------------
 * This program attaches an eBPF program to a tracepoint, fentry, fexit, kprobe, or kretprobe.
 * It supports user-specified or default BPF object files (.o) and function names.
 *
 * Command Syntax:
 *   ./bpf_attach_prog <type> [<bpf_file>] [<function_name>]
 *
 * Parameters:
 * - <type>: The type of attachment. Can be one of the following:
 *     - 'tp/<category>/<event>': Attach to a tracepoint, where <category> and <event> define the tracepoint.
 *     - 'fentry/<function>': Attach to the entry of a specific kernel function.
 *     - 'fexit/<function>': Attach to the exit of a specific kernel function.
 *     - 'kprobe/<function>': Attach to the entry of a specific kernel function.
 *     - 'kretprobe/<function>': Attach to the return of a specific kernel function.
 *
 * - [<bpf_file>] (optional): The eBPF object file (.o) containing the eBPF program.
 *   - Default for tracepoint: "tracepoint.kern.o"
 *   - Default for fentry: "fentry.kern.o"
 *   - Default for fexit: "fexit.kern.o"
 *   - Default for kprobe: "kprobe.kern.o"
 *   - Default for kretprobe: "kretprobe.kern.o"
 *
 * - [<function_name>] (optional): The eBPF function name to attach. Defaults to "empty" if not provided.
 *
 * Example Commands:
 * -----------------
 * 1. Attach to a tracepoint with defaults:
 *    ./link tp/syscalls/sys_enter_getcwd
 *    (Uses: tracepoint.kern.o and function "empty")
 *
 * 2. Attach to a tracepoint with a custom BPF file and function:
 *    ./link tp/syscalls/sys_enter_getcwd custom_tracepoint.o empty
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <linux/limits.h>

#define PIN_PATH "/sys/fs/bpf"

// Default BPF object files for each hook type
#define DEFAULT_TRACEPOINT_BPF_FILE "tracepoint.kern.o"
#define DEFAULT_FENTRY_BPF_FILE "fentry.kern.o"
#define DEFAULT_FEXIT_BPF_FILE "fexit.kern.o"
#define DEFAULT_KPROBE_BPF_FILE "kprobe.kern.o"
#define DEFAULT_KRETPROBE_BPF_FILE "kretprobe.kern.o"

// Default function name for all hook types
#define DEFAULT_FUNC_NAME "empty"

void usage(const char *prog) {
    fprintf(stderr, "Usage: %s <type> [<bpf_file>] [<function_name>]\n", prog);
    fprintf(stderr, "Example for tracepoint: %s tp/syscalls/sys_enter_getcwd [tracepoint.kern.o] [empty]\n", prog);
    fprintf(stderr, "Example for fentry: %s fentry/do_unlinkat [fentry.kern.o] [empty]\n", prog);
    fprintf(stderr, "Example for fexit: %s fexit/do_unlinkat [fexit.kern.o] [empty]\n", prog);
    fprintf(stderr, "Example for kprobe: %s kprobe/do_unlinkat [kprobe.kern.o] [empty]\n", prog);
    fprintf(stderr, "Example for kretprobe: %s kretprobe/do_unlinkat [kretprobe.kern.o] [empty]\n", prog);
    exit(EXIT_FAILURE);
}

int load_and_attach_bpf(const char *prog_name, const char *obj_file, struct bpf_link **link, const char *attach_type, const char *category, const char *target) {
    struct bpf_object *obj;
    struct bpf_program *prog;

    // Load the eBPF object file
    obj = bpf_object__open(obj_file);
    if (libbpf_get_error(obj)) {
        fprintf(stderr, "Error opening eBPF object file: %s\n", obj_file);
        return -1;
    }

    // Find the eBPF program by function name
    prog = bpf_object__find_program_by_name(obj, prog_name);
    if (!prog) {
        fprintf(stderr, "Error finding eBPF program: %s\n", prog_name);
        bpf_object__close(obj);
        return -1;
    }

    // Load the eBPF program into the kernel
    if (bpf_object__load(obj)) {
        fprintf(stderr, "Error loading eBPF program\n");
        bpf_object__close(obj);
        return -1;
    }

    // Attach the program based on type (tracepoint, fentry, fexit, kprobe, kretprobe)
    if (strcmp(attach_type, "tracepoint") == 0) {
        *link = bpf_program__attach_tracepoint(prog, category, target);
    } else if (strcmp(attach_type, "fentry") == 0 || strcmp(attach_type, "fexit") == 0) {
       // Inform the user to modify the SEC() section manually
        printf("Ensure that your eBPF program has the correct SEC() section for fentry or fexit hooks as libbpf does not support custom hooking.\n");
        printf("For example, use SEC(\"fentry/function_name\") for function entry and SEC(\"fexit/function_name\") for function exit.\n");

        // Use the generic bpf_program__attach function for fentry and fexit
        *link = bpf_program__attach(prog);
        
    } else if (strcmp(attach_type, "kprobe") == 0) {
        *link = bpf_program__attach_kprobe(prog, false, target);  // kprobe, not kretprobe
    } else if (strcmp(attach_type, "kretprobe") == 0) {
        *link = bpf_program__attach_kprobe(prog, true, target);   // kretprobe
    }

    if (libbpf_get_error(*link)) {
        fprintf(stderr, "Error attaching to %s '%s'\n", attach_type, target);
        bpf_object__close(obj);
        return -1;
    }

    bpf_object__close(obj);
    return 0;
}

int pin_bpf_program(const char *path, int prog_fd) {
    if (bpf_obj_pin(prog_fd, path) < 0) {
        fprintf(stderr, "Error pinning the eBPF program to '%s': %s\n", path, strerror(errno));
        return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    char pin_path[PATH_MAX];
    char *input, *category, *event, *function, *bpf_file;
    struct bpf_link *link = NULL;
    int prog_fd;

    if (argc < 2) {
        usage(argv[0]);
    }

    input = argv[1];

    // Determine which hook type we're working with
    if (strncmp(input, "tp/", 3) == 0) {
        // Handle tracepoint
        category = strtok(input + 3, "/");
        event = strtok(NULL, "/");
        if (!category || !event) {
            fprintf(stderr, "Error: Invalid tracepoint format. Expecting 'tp/<category>/<event>'.\n");
            return 1;
        }
        printf("Attaching to tracepoint category: %s, event: %s\n", category, event);

        // Set default or user-provided BPF file and function name
        bpf_file = (argc > 2) ? argv[2] : DEFAULT_TRACEPOINT_BPF_FILE;
        function = (argc > 3) ? argv[3] : DEFAULT_FUNC_NAME;

        // Load, attach, and pin
        if (load_and_attach_bpf(function, bpf_file, &link, "tracepoint", category, event) < 0) {
            return 1;
        }
        snprintf(pin_path, sizeof(pin_path), "%s/%s_%s", PIN_PATH, category, event);

    } else if (strncmp(input, "fentry/", 7) == 0) {
        // Handle fentry
        function = input + 7;
        printf("Attaching to fentry function: %s\n", function);

        // Set default or user-provided BPF file and function name
        bpf_file = (argc > 2) ? argv[2] : DEFAULT_FENTRY_BPF_FILE;
        function = (argc > 3) ? argv[3] : DEFAULT_FUNC_NAME;

        if (load_and_attach_bpf(function, bpf_file, &link, "fentry", "empty", function) < 0) {
            return 1;
        }
        snprintf(pin_path, sizeof(pin_path), "%s/fentry_%s", PIN_PATH, function);

    } else if (strncmp(input, "fexit/", 6) == 0) {
        // Handle fexit
        function = input + 6;
        printf("Attaching to fexit function: %s\n", function);

        // Set default or user-provided BPF file and function name
        bpf_file = (argc > 2) ? argv[2] : DEFAULT_FEXIT_BPF_FILE;
        function = (argc > 3) ? argv[3] : DEFAULT_FUNC_NAME;

        if (load_and_attach_bpf(function, bpf_file, &link, "fexit", "empty", function) < 0) {
            return 1;
        }
        snprintf(pin_path, sizeof(pin_path), "%s/fexit_%s", PIN_PATH, function);

    } else if (strncmp(input, "kprobe/", 7) == 0) {
        // Handle kprobe
        function = input + 7;
        printf("Attaching to kprobe function: %s\n", function);

        // Set default or user-provided BPF file and function name
        bpf_file = (argc > 2) ? argv[2] : DEFAULT_KPROBE_BPF_FILE;
        function = (argc > 3) ? argv[3] : DEFAULT_FUNC_NAME;

        if (load_and_attach_bpf(function, bpf_file, &link, "kprobe", "empty", function) < 0) {
            return 1;
        }
        snprintf(pin_path, sizeof(pin_path), "%s/kprobe_%s", PIN_PATH, function);

    } else if (strncmp(input, "kretprobe/", 10) == 0) {
        // Handle kretprobe
        function = input + 10;
        printf("Attaching to kretprobe function: %s\n", function);

        // Set default or user-provided BPF file and function name
        bpf_file = (argc > 2) ? argv[2] : DEFAULT_KRETPROBE_BPF_FILE;
        function = (argc > 3) ? argv[3] : DEFAULT_FUNC_NAME;

        if (load_and_attach_bpf(function, bpf_file, &link, "kretprobe", "empty", function) < 0) {
            return 1;
        }
        snprintf(pin_path, sizeof(pin_path), "%s/kretprobe_%s", PIN_PATH, function);

    } else {
        fprintf(stderr, "Error: Unsupported format. Use 'tp/<category>/<event>' for tracepoints, 'fentry/<function>' for fentry hooks, 'fexit/<function>' for fexit hooks, 'kprobe/<function>' for kprobes, or 'kretprobe/<function>' for kretprobes.\n");
        usage(argv[0]);
    }

    // Pin the BPF program
    prog_fd = bpf_link__fd(link);
    if (pin_bpf_program(pin_path, prog_fd) < 0) {
        return 1;
    }

    printf("Successfully loaded, attached, and pinned eBPF program to %s\n", pin_path);

    // Clean up
    // bpf_link__destroy(link);
    return 0;
}
