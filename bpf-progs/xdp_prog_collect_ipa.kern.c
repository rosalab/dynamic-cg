#include <linux/bpf.h>
#include <linux/types.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>
#include <linux/if_ether.h>
#include <linux/ip.h>

#define MAX_ENTRIES 100000

struct { 
    __uint(type, BPF_MAP_TYPE_HASH);
	__type(key, __be32);
	__type(value, __u64);
	__uint(max_entries, MAX_ENTRIES);
} ip_map SEC(".maps");

SEC("xdp")
int trigger_xdp_prog(struct xdp_md *ctx) 
{
	void *data_end = (void *)(long)ctx->data_end;
	void *data     = (void *)(long)ctx->data;

    struct ethhdr *eth = data;

    if ((void *)eth + sizeof(struct ethhdr) > data_end) {
        return XDP_ABORTED;    
    }

    if (eth->h_proto != bpf_htons(ETH_P_IP)) {
        return XDP_PASS;
    }

    struct iphdr *ip = data + sizeof(struct ethhdr);

    if ((void *)ip + sizeof(struct iphdr) > data_end) {
        return XDP_ABORTED;
    }
    
    __be32 src_ip = ip->saddr;
   // __be32 dst_ip = ip->daddr;

   // bpf_printk("protocol: %u", ip->protocol);
   // bpf_printk("src ip addr: %u.%u.%u.%u\n", bpf_ntohs(src_ip) >> 8 & 0xFF,
   //                                          bpf_ntohs(src_ip) & 0xFF,
   //                                          bpf_ntohs(src_ip >> 16) >> 8 & 0xFF,
   //                                          bpf_ntohs(src_ip >> 16) & 0xFF);
   // bpf_printk("dst ip addr: %u.%u.%u.%u\n", bpf_ntohs(dst_ip) >> 8 & 0xFF,
   //                                          bpf_ntohs(dst_ip) & 0xFF,
   //                                          bpf_ntohs(dst_ip >> 16) >> 8 & 0xFF,
   //                                          bpf_ntohs(dst_ip >> 16) & 0xFF);

    __u64 *val = bpf_map_lookup_elem(&ip_map, &src_ip);

    if (!val) {
        __u64 tmp = 0;
        int ret = bpf_map_update_elem(&ip_map, &src_ip, &tmp, BPF_ANY);
        if (ret < 0) {
            bpf_printk("Map update failed\n");
        }
        return XDP_PASS;
    }

    __sync_fetch_and_add(val, 1);

//    bpf_printk("src ip addr: %u.%u.%u.%u and val is %lu\n", bpf_ntohs(src_ip) >> 8 & 0xFF,
//                                             bpf_ntohs(src_ip) & 0xFF,
//                                             bpf_ntohs(src_ip >> 16) >> 8 & 0xFF,
//                                             bpf_ntohs(src_ip >> 16) & 0xFF, *val);

	return XDP_PASS;
}

char LISENSE[] SEC("license") = "Dual BSD/GPL";
