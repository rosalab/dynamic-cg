
#include <linux/bpf.h>
#include <linux/types.h>
#include <bpf/bpf_helpers.h>

SEC("fentry/__x64_sys_socket")
int trigger_bpf_prog(void *ctx) {
    bpf_printk("Triggered socket syscall\n");
    return 0;
}

char LISENSE[] SEC("license") = "Dual BSD/GPL";
