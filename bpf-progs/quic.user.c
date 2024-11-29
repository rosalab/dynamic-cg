#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <unistd.h>
#include<linux/if_link.h>

#define MAX_CID_LEN 4

int main(int argc, char **argv) {
    const char *obj_file = "extract.CID.kern.o"; // Kernel program object file
    const char *iface = "eth0"; // Change to your desired interface name
    struct bpf_object *obj;
    struct bpf_program *prog;
    int prog_fd, iface_index;

    // Verify the network interface
    iface_index = if_nametoindex(iface);
    if (!iface_index) {
        fprintf(stderr, "ERROR: Invalid interface name: %s\n", iface);
        return 1;
    }

    printf("Loading BPF object from file: %s\n", obj_file);

    // Open the BPF object file
    obj = bpf_object__open_file(obj_file, NULL);
    if (libbpf_get_error(obj)) {
        fprintf(stderr, "ERROR: Failed to open BPF object file: %s\n", strerror(-libbpf_get_error(obj)));
        return 1;
    }

    printf("****************************************************WE ARE HEREEE\n");
    // Load the BPF program into the kernel
    if (bpf_object__load(obj)) {
        fprintf(stderr, "ERROR: Failed to load BPF object file: %s\n", strerror(errno));
        bpf_object__close(obj);
        return 1;
    }


    // Find the XDP program by its section name
    prog = bpf_object__find_program_by_name(obj, "extract_quic_cid");
    if (!prog) {
        fprintf(stderr, "ERROR: Failed to find BPF program in object file\n");
        bpf_object__close(obj);
        return 1;
    }

    // Get the program file descriptor
    prog_fd = bpf_program__fd(prog);
    if (prog_fd < 0) {
        fprintf(stderr, "ERROR: Failed to get BPF program FD\n");
        bpf_object__close(obj);
        return 1;
    }

//    // Attach the XDP program to the interface
//    if (bpf_xdp_attach(iface_index, prog_fd, XDP_FLAGS_UPDATE_IF_NOEXIST, NULL) < 0) {
//        fprintf(stderr, "ERROR: Failed to attach XDP program to interface %s: %s\n", iface, strerror(errno));
//        bpf_object__close(obj);
//        return 1;
//    }

    // Attach the XDP program to the network interface
    if (bpf_xdp_attach(iface_index, prog_fd, XDP_FLAGS_UPDATE_IF_NOEXIST, NULL) < 0) {
        fprintf(stderr, "ERROR: Failed to attach XDP program to interface %s\n", iface);
        bpf_object__close(obj);
        return 1;
    }

    printf("XDP program successfully attached to interface %s!\n", iface);

    // Wait indefinitely to keep the program running
//    printf("Press Ctrl+C to exit...\n");
//    while (1) {
//        sleep(1);
//    }

    struct bpf_map *map;
    int map_fd;

    // Find and get map file descriptor
    map = bpf_object__find_map_by_name(obj, "cid_map");
    if (!map) {
        fprintf(stderr, "ERROR: Map 'cid_map' not found in BPF object\n");
        return 1;
    }

    // Get the file descriptor of the map
    map_fd = bpf_map__fd(map);
    if (map_fd < 0) {
        fprintf(stderr, "ERROR: Failed to get file descriptor for map\n");
        return 1;
    }

    printf("Successfully retrieved map FD: %d\n", map_fd);

    // Lookup a specific key in the map
    unsigned char test_cid[MAX_CID_LEN] = {0xde, 0xad, 0xbe, 0xef};

    int backend_id;

    // Lookup a specific key in the map and print the key
    printf("Looking up map with test_cid: %02x %02x %02x %02x, map_fd: %d\n",
        test_cid[0], test_cid[1], test_cid[2], test_cid[3], map_fd);

    if (bpf_map_lookup_elem(map_fd, test_cid, &backend_id) == 0) {
        printf("Backend ID for test CID is: %d\n", backend_id);
    } else {
        printf("Test CID not found in map: %d\n", errno);
    }
//    printf("Backend ID for test CID is: %d\n", backend_id);
//    printf("Backend ID as unsigned int: %u\n", (unsigned int) backend_id);
//    printf("Backend ID as hex: 0x%x\n", backend_id);


    // Iterate through all keys in the map
//    unsigned char next_cid[MAX_CID_LEN] = {0};
//    unsigned char prev_cid[MAX_CID_LEN] = {0};
//    int first_key = 1;
//
//    while (bpf_map_get_next_key(map_fd, first_key ? NULL : prev_cid, next_cid) == 0) {
//        if (bpf_map_lookup_elem(map_fd, next_cid, &backend_id) == 0) {
//            printf("CID: ");
//            for (int i = 0; i < MAX_CID_LEN; i++) {
//                printf("%02x", next_cid[i]);
//            }
//            printf(" -> Backend ID: %d\n", backend_id);
//        }
//        memcpy(prev_cid, next_cid, MAX_CID_LEN);
//        first_key = 0;
//    }

//    if (bpf_xdp_attach(iface_index, -1, 0) < 0) {
//            fprintf(stderr, "ERROR: Failed to detach BPF program from interface %s\n", iface);
//            return 1;
//        }
//    printf("XDP program detached from interface %s\n", iface);

    // Cleanup (this code will not execute unless you add signal handling)
    bpf_object__close(obj);
    return 0;
}
