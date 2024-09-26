#include <linux/bpf.h>
#include <linux/types.h>
#include <bpf/bpf_helpers.h>

SEC("fentry/__alloc_skb")
int trigger_syscall_prog(void *ctx)
{
    __u32 pid = bpf_get_current_pid_tgid() >> 32;
    bpf_printk("%d\n", pid);
    return 0;
}

char LISENSE[] SEC("license") = "Dual BSD/GPL";

