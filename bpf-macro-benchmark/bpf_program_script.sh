#!/usr/bin/env bash
# script to link and load ebpf programs

cd ./bpf-programs

eval './link.user fentry/__alloc_skb tp.kern.o empty'
