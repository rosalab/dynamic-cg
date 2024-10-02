
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>


int target_pid = 370;
void *user_ptr = (void *)0x7ffc754c29b0;

SEC("fexit.s/_raw_spin_lock")
int trigger_bpf_prog(void *ctx) {
    struct task_struct *task = bpf_get_current_task_btf();

    if(task->pid != target_pid) {
        return 0; 
    }
    bpf_printk("Triggered socket syscall\n");

    char buf[8];

    int ret = bpf_copy_from_user_task(buf, sizeof(buf), user_ptr, task, 0);

    bpf_printk("printing userspace value %lx\n", (unsigned long)buf);
    bpf_printk("Return Value from the helper function: %d\n", ret);


    return 0;
}

char LISENSE[] SEC("license") = "Dual BSD/GPL";
