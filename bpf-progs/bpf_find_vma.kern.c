
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>


struct callback_ctx {
	int dummy;
};

#define VM_EXEC		0x00000004
#define DNAME_INLINE_LEN 32

char d_iname[DNAME_INLINE_LEN] = {0};
__u32 found_vm_exec = 0;
__u64 addr = 0x55ba2a20e000; // replace this manually
int find_zero_ret = -1;
int find_addr_ret = -1;


static long check_vma(struct task_struct *task, struct vm_area_struct *vma,
		      struct callback_ctx *data)
{
    bpf_printk("Callback function is triggered\n");
	if (vma->vm_file)
		bpf_probe_read_kernel_str(d_iname, DNAME_INLINE_LEN - 1,
					  vma->vm_file->f_path.dentry->d_iname);

	/* check for VM_EXEC */
	if (vma->vm_flags & VM_EXEC)
		found_vm_exec = 1;

	return 0;
}

SEC("fentry/__x64_sys_socket")
int trigger_bpf_prog(void *ctx) {
    bpf_printk("Triggered socket syscall\n");
    struct task_struct *task = bpf_get_current_task_btf();
	struct callback_ctx data = {};

	find_addr_ret = bpf_find_vma(task, addr, check_vma, &data, 0);
    bpf_printk("printing the variables\n find_add_ret %d\nfind_zero_ret %d\n found_vm_exec %d\n", find_addr_ret, find_zero_ret, found_vm_exec);

	/* this should return -ENOENT */
	find_zero_ret = bpf_find_vma(task, 0, check_vma, &data, 0);
	
    bpf_printk("printing the variables\n find_add_ret %d\nfind_zero_ret %d\n found_vm_exec %d\n", find_addr_ret, find_zero_ret, found_vm_exec);

    return 0;
}

char LISENSE[] SEC("license") = "Dual BSD/GPL";
