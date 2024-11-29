#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/in.h>
#include <bpf/bpf_helpers.h>

#define MAX_CID_LEN 20
#define QUIC_LONG_HEADER 0b11000000

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 1024);
    __type(key, unsigned char[MAX_CID_LEN]); // Key type: CID (array of bytes)
    __type(value, int);                      // Value type: Backend server ID
    __uint(pinning, 1);
} cid_map SEC(".maps");

SEC("xdp")
int extract_quic_cid(struct xdp_md *ctx) {
    void *data = (void *)(long)ctx->data;
    void *data_end = (void *)(long)ctx->data_end;

    // Ethernet header
    struct ethhdr *eth = data;
    if ((void *)eth + sizeof(*eth) > data_end)
        return XDP_ABORTED;

    // Check if the packet is IPv4
    if (eth->h_proto != __constant_htons(ETH_P_IP))
        return XDP_PASS;

    // IPv4 header
    struct iphdr *ip = (void *)eth + sizeof(*eth);
    if ((void *)ip + sizeof(*ip) > data_end || ip->protocol != IPPROTO_UDP)
        return XDP_PASS;

    // UDP header
    struct udphdr *udp = (void *)ip + sizeof(*ip);
    if ((void *)udp + sizeof(*udp) > data_end)
        return XDP_PASS;

    // Payload starts after the UDP header
    unsigned char *payload = (unsigned char *)udp + sizeof(*udp);
    if ((void *)payload + 7 > data_end)
        return XDP_PASS;

    // Ensure this is a QUIC long header packet
    if ((payload[0] & QUIC_LONG_HEADER) != QUIC_LONG_HEADER)
        return XDP_PASS;

    // Get the CID length
    unsigned char cid_len = payload[6];

    // Ensure that there is enough space in the packet for the CID
    if (cid_len == 0 || cid_len > MAX_CID_LEN || (void *)payload + 7 + cid_len > data_end)
        return XDP_PASS;

    // Copy CID into a buffer
    unsigned char cid[MAX_CID_LEN] = {0};
    for (int i = 0; i < cid_len; i++) {
        if ((void *)(payload + 7 + i) >= data_end) {
            return XDP_ABORTED; // Safety check for each byte
        }
        cid[i] = payload[7 + i];
    }

    // Backend server ID
    int backend_id = 1;

    if (bpf_map_update_elem(&cid_map, cid, &backend_id, BPF_ANY) < 0) {
        bpf_printk("ERROR: Failed to update map with test CID\n");
        return XDP_ABORTED; // Log error if map update fails
    } else {
        bpf_printk("Successfully updated map with test CID");
    }

    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";