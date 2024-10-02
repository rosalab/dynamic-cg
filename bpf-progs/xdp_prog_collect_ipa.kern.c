#include <linux/bpf.h>
#include <linux/types.h>
#include <bpf/bpf_helpers.h>

SEC("xdp")
int trigger_xdp_prog(struct xdp_md *ctx) 
{
	void *data_end = (void *)(long)ctx->data_end;
	void *data     = (void *)(long)ctx->data;
	
	return XDP_PASS;
}

char LISENSE[] SEC("license") = "Dual BSD/GPL";
