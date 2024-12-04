#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

// Metadata for network packets
struct __attribute__((btf_type_tag("sensitivity:low"))) packet_meta {
    char data[256]; // Packet metadata (low sensitivity by default)
    __u8 flags;     // Flags for internal use
};

struct __attribute__((btf_type_tag("sensitivity:high"))) sensitive_data {
    char secret[64]; // Sensitive payload
};

// BPF map for logging metadata
struct {
    __uint(type, BPF_MAP_TYPE_PERCPU_ARRAY);
    __uint(max_entries, 1024);
    __type(key, __u32);
    __type(value, struct packet_meta);
} log_map SEC(".maps");

SEC("xdp")
int inspect_packet(struct xdp_md *ctx) {
    void *data = (void *)(long)ctx->data;
    void *data_end = (void *)(long)ctx->data_end;

    if (data + sizeof(struct ethhdr) > data_end)
        return XDP_PASS;

    // Extract non-sensitive packet metadata
    struct packet_meta meta = {};
    bpf_probe_read(&meta, sizeof(meta), data);

    // Extract sensitive data (hypothetical condition)
    struct sensitive_data secret = {};
    if (data + sizeof(struct ethhdr) + sizeof(struct sensitive_data) > data_end)
        return XDP_PASS;

    bpf_probe_read(&secret, sizeof(secret), data + sizeof(struct ethhdr));

    // Attempt to log sensitive data (verifier should block this)
    __u32 key = 0;

    // Should pass: logging low-sensitivity data
    bpf_map_update_elem(&log_map, &key, &meta, BPF_ANY);

    // Should fail: logging high-sensitivity data
    bpf_map_update_elem(&log_map, &key, &secret, BPF_ANY);

    return XDP_PASS;
}
