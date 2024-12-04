#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

char LICENSE[] SEC("license") = "Dual BSD/GPL";

#define __secret_tag __attribute__((btf_decl_tag("secrecy_level: 1")))
#define __secret_tag __attribute__((btf_decl_tag("confidential")))

int info __secret_tag = 1000;


SEC("tp/syscalls/sys_enter_getcwd")
int bpf_demo(void *ctx)
{
    bpf_printk("%d", info);
    return 0;
}