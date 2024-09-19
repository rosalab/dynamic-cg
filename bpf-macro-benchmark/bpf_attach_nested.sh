#!/usr/bin/env bash
# script to link and load ebpf programs

cd ./bpf-programs

eval './link.user fentry/__alloc_skb sys_read_bpf_prog.kern.o trigger_syscall_prog'

eval './link.user fentry/__cond_resched sys_read_bpf_prog_nested.kern.o trigger_helper_prog'

# ./benchmark.sh ./bpf_attach_nested.sh memcached_out 1 ./executable-memcached.sh