#include <linux/bpf.h>
#include <linux/types.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>
#include <linux/if_ether.h>
#include <linux/ip.h>

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
    __be32 dst_ip = ip->daddr;

    bpf_printk("protocol: %u", ip->protocol);
    bpf_printk("src ip addr: %u.%u.%u.%u\n", bpf_ntohs(src_ip) >> 24 & 0xFF,
                                             (bpf_ntohs(src_ip) >> 16) & 0xFF,
                                             (bpf_ntohs(src_ip) >> 8) & 0xFF,
                                             bpf_ntohs(src_ip) & 0xFF);
    bpf_printk("dst ip addr: %u.%u.%u.%u\n", bpf_ntohs(dst_ip) >> 24 & 0xFF,
                                             (bpf_ntohs(dst_ip) >> 16) & 0xFF,
                                             (bpf_ntohs(dst_ip) >> 8) & 0xFF,
                                             bpf_ntohs(dst_ip) & 0xFF);
	return XDP_PASS;
}

char LISENSE[] SEC("license") = "Dual BSD/GPL";
