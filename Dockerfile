FROM ubuntu:24.04 as Linux-builder

ENV LINUX=/linux 

RUN DEBIAN_FRONTEND=noninteractive apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get upgrade -y
RUN DEBIAN_FRONTEND=noninteractive apt-get install --fix-missing -y git build-essential gcc g++ fakeroot libncurses5-dev libssl-dev ccache dwarves libelf-dev \
 cmake mold \
 libdw-dev libdwarf-dev \
 bpfcc-tools libbpfcc-dev libbpfcc \
 linux-headers-generic \
 libtinfo-dev \
 libstdc++-11-dev libstdc++-12-dev \
 bc \
 flex bison \
 rsync \
 libcap-dev libdisasm-dev binutils-dev unzip \
 pkg-config lsb-release wget software-properties-common gnupg zlib1g llvm \
 qemu-kvm libvirt-clients libvirt-daemon-system bridge-utils virtinst libvirt-daemon xterm attr busybox openssh-server \
 iputils-ping kmod

# perf dependencies
RUN DEBIAN_FRONTEND=noninteractive apt-get install --fix-missing -y \
    clang \
    libslang2-dev \
    systemtap-sdt-dev \
    libunwind-dev \
    libbabeltrace-dev \
    libtraceevent-dev \
    libperl-dev \
    python3-dev

# Update package list and install memcached, memtier_benchmark and required tools
RUN apt-get update && \
apt-get install -y trace-cmd memcached git build-essential autoconf automake libevent-dev libpcre3-dev && \
apt-get clean && \
rm -rf /var/lib/apt/lists/*

# Install memtier_benchmark
RUN git clone https://github.com/RedisLabs/memtier_benchmark.git && \
cd memtier_benchmark && \
autoreconf -ivf && \
./configure && \
make && \
make install

# Expose memcached default port
EXPOSE 11211