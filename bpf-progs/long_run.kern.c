#include <linux/bpf.h>
#include <linux/types.h>
#include <bpf/bpf_helpers.h>

#define LEFT_SHIFT 5

static int runner3(void *ctx) {
    //bpf_printk("runner: Inside 3 loop function\n");

    return 0;
}

static int runner2(void *ctx) {
    //bpf_printk("runner: Inside 2 loop function\n");
    __u32 iter = (1<<LEFT_SHIFT);

    bpf_loop(iter, runner3, NULL, 0);

    return 0;
}

static int runner(void *ctx) {
    //bpf_printk("runner: Inside 1 loop function\n");
    __u32 iter = (1<<LEFT_SHIFT);

    bpf_loop(iter, runner2, NULL, 0);

    return 0;
}

SEC("fentry/__htab_map_lookup_elem")
int trigger_helper_prog(void *ctx) {

    //bpf_printk("trigger_helper_prog: Test BPF Loop Program\n");

    __u32 iter = (1<<LEFT_SHIFT);

    bpf_loop(iter, runner, NULL, 0);

    return 0;

}

char LISENSE[] SEC("license") = "Dual BSD/GPL";

