#!/usr/bin/env bash
# script to link and load ebpf programs

cd ./bpf-programs

eval './link.user tp/syscalls/sys_enter_getcwd tp.kern.o empty'
