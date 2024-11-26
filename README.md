# Linux Development Environment
This repository contains *one* workflow for building and modifying the Linux kernel.
It consists of two main components.
The first is a docker container that contains all the requirements to build the Linux kernel, as well
    as the requirements to run QEMU.
The second is a QEMU  script that boots a virtual machine running a custom version of the Linux kernel.
Using these together allows you to easily make and test changes to the Linux kernel without needing to
    manage all the packages locally.

***This repository is cloned and modified from rosalab/(unknown)-kernel***

#### Instructions specific to the termination project (skip if you're looking for the setup instructions)

 - Compile the kernel using the working_config.config
    ```sh
    cp working_config.config .config
    ```
    To verify, check .config to have CONFIG_HAVE_BPF_TERMINATION=y
 - Change directory to `bpf-progs`
    ```sh
    cd bpf-progs
    ```
 - Build
    ```sh
    make
    ```
 - Run the userspace program to attach the BPF program
    ```sh
    ./loops.user
    ```
    Verify if the BPF program is attached using
    ```sh
    bpftool prog show
    ```
 - Trigger the BPF program
    ```sh
    ./trigger_loops.user
    ```
 - Terminate the program using the program ID found before.
    ```sh
    bpftool prog terminate <prog_id>
    ```
    Check `dmesg` for similar logs as mentioned below:

    ```sh
    [  296.332446][  T296] Starting terminate syscall prog_id : 5 at time : 296325433601
    [  296.334733][  T296] Finished termination syscall at time : 296327720324
    [  296.336654][  T296] IPI + Termination handler took : 2286723 ns
    [  299.395261][  T297] processed 2 insns (limit 1000000) max_states_per_insn 0 total_states 0 peak_states 0 mark_read 0
    [  299.401528][  T297] processed 2 insns (limit 1000000) max_states_per_insn 0 total_states 0 peak_states 0 mark_read 0
    [  299.404505][  T297] processed 2 insns (limit 1000000) max_states_per_insn 0 total_states 0 peak_states 0 mark_read 0
    [  302.135727][  T298] processed 2 insns (limit 1000000) max_states_per_insn 0 total_states 0 peak_states 0 mark_read 0
    [  302.142161][  T298] processed 2 insns (limit 1000000) max_states_per_insn 0 total_states 0 peak_states 0 mark_read 0
    [  302.145287][  T298] processed 2 insns (limit 1000000) max_states_per_insn 0 total_states 0 peak_states 0 mark_read 0
    [  407.147189][  T311] Entering bpf_get_numa_node_id at time : 407140176600
    [  417.149818][  T311] Exiting bpf_get_numa_node_id at time : 417142805869
    [  417.150385][  T311] Entering bpf_get_numa_node_id at time : 417143373453
    [  421.519914][  T312] Starting terminate syscall prog_id : 5 at time : 421512901566
    [  421.522232][    C3] kernel/smp.c sync call to bpf_die
    [  421.522683][    C3] bpf_die called on [CPU:3] prog:5
    [  421.523091][    C3] [kernel/bpf/syscall.c:215] Found old_addr to be in bpf_func
    [  421.523091][    C3] Adding offset:0x2c to dest addr : 0xffffffffa0000674
    [  421.524177][  T312] Finished termination syscall at time : 421517164842
    [  421.526034][  T312] IPI + Termination handler took : 4263276 ns
    [  427.153667][  T311] Exiting bpf_get_numa_node_id at time : 427146654583
    [  427.154251][  T311] Exiting bpf_prog_run at time : 427147239128
    [  445.732480][  T313] processed 2 insns (limit 1000000) max_states_per_insn 0 total_states 0 peak_states 0 mark_read 0
    [  445.739202][  T313] processed 2 insns (limit 1000000) max_states_per_insn 0 total_states 0 peak_states 0 mark_read 0
    [  445.742556][  T313] processed 2 insns (limit 1000000) max_states_per_insn 0 total_states 0 peak_states 0 mark_read 
    ```

#### Build Docker Container

``` make docker ```

#### Update git submodules
The `linux` directory contains a forked linux kernel source tree as a git submodule. The below commands help you to update it.

```sh
git submodule init

# This will take some time.
git submodule update
```

#### Copy config file to linux folder

``` cp linux-config/.config ./linux ```

or 

```
cd linux
make defconfig
cd ../
```

#### Build linux

```
make vmlinux
```

#### Run Qemu
```
make qemu-run
```

#### If you want to ssh into the qemu
```
make qemu-ssh
```

#### If you want to enter the docker container where qemu is running
```
make enter-docker
```

#### If you want to debug the kernel using gdb
```
make qemu-run-gdb
```
In an another terminal
```
cd linux
gdb vmlinux
target remote:1234
c
```
then set your breakpoints and debug more


## Adding Ports to QEMU
By default host port 52223 is connected to port 52223 inside the QEMU virtual machine.
If you need to be able to connect to more than one port (or a specific port) on your custom kernel from the host, you will have to add new rules.
The needed rules are in `q-script/yifei-q` and in the Makefile.

### Makefile modifications
You must add a line that maps a host port to a Docker port.
In the Makefile you must add a line 
    ```-p 127.0.0.1:HOST_PORT:DOCKER_PORT```
This will map the host port to the docker port.

### q-script modifications
You must modify the q-script to connect the DOCKER_PORT to a QEMU_PORT.
In the q-script you must append a new rule.
Find the line that starts with `"net += -netdev user..."`.
Then at the end of the line add the text ```"hostfwd=tcp::DOCKER_PORT-:QEMU_PORT"```


```
scp -P 50004 -O root@127.0.0.1:/var/log/system_healthcheck.log .

scp rahult@miles.cs.vt.edu:/home/rahult/linux-dev-env/system_healthcheck.log /Users/rahul/Documents/os/
```