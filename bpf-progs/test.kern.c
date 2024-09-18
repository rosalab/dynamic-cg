#include <linux/bpf.h>
#include <linux/types.h>
#include <bpf/bpf_helpers.h>

SEC("fentry/__x64_sys_socket")
int trigger_syscall_prog(void *ctx) {

      bpf_printk("trigger_syscall_prog: Test BPF Program\n");

    return 0;

}

char LISENSE[] SEC("license") = "Dual BSD/GPL";

