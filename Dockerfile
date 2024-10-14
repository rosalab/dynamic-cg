FROM ubuntu:24.04 AS egor-linux-dev

ENV LINUX=/linux 

# Set non-interactive mode for apt
ARG DEBIAN_FRONTEND=noninteractive

# nginx
ENV NGINX_VERSION=1.26.1 \
    NGINX_RUN=/usr/local/nginx \
    NGINX_BIN=/usr/local/sbin/nginx \
    NGINX_CONF=/usr/local/nginx/conf


RUN apt-get update && apt-get upgrade -y && \
    apt-get install -y \
    git build-essential gcc g++ \
    curl wget unzip vim


RUN apt-get update && apt-get install -y  --no-install-recommends \
    fakeroot libncurses5-dev libssl-dev libncurses-dev \
    ccache dwarves libelf-dev cmake libdw-dev libdwarf-dev \
    bpfcc-tools libbpfcc-dev linux-headers-generic \
    libtinfo-dev libstdc++-13-dev libstdc++-14-dev bc \
    flex bison rsync kmod \
    libcap-dev libdisasm-dev binutils-dev \
    pkg-config lsb-release gnupg \
    zlib1g zlib1g-dev pciutils memcached \
    libmemcached-tools libevent-dev inetutils-tools \
    iputils-ping libpcre3 libpcre3-dev libgd-dev \
    libxml2 libxml2-dev uuid-dev xdp-tools wrk \
    iproute2 net-tools lsof hping3 apt-utils \
    autoconf automake sysbench fio dstat \
    htop sysstat linux-tools-generic binutils \
    python3 python3-pip python3-venv bonnie++ \
    cmake-curses-gui hardinfo snap nano man-db \
    iputils-arping iputils-ping iputils-tracepath \
    rclone bpftrace strace tmux gdb p7zip clang llvm  \
    apache2-utils tar redis-server fping netcat-traditional wireshark

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


# RUN apt-get update && \
#     apt-get install -y curl && \
#     curl -fsSL https://raw.github.com/jeschu/mtr/master/install-mtr.sh -o install-mtr.sh && \
#     sed -i 's/sudo //g' install-mtr.sh && \
#     sh install-mtr.sh && \
#     apt-get clean && rm -rf /var/lib/apt/lists/*

# Create necessary directories
# RUN mkdir -p /downloads_nginx /memcached/build /memcached/extract /opt/venv
# RUN mkdir -p /downloads_nginx /opt/venv

# Download, extract, configure, and install NGINX
# RUN cd /downloads_nginx \
#     && wget https://nginx.org/download/nginx-${NGINX_VERSION}.tar.gz \
#     && tar xvzf ./nginx-${NGINX_VERSION}.tar.gz \
#     && cd nginx-${NGINX_VERSION} \
#     && ./configure \
#         --sbin-path=/usr/sbin/nginx \
#         --conf-path=/etc/nginx/nginx.conf \
#         --pid-path=/var/run/nginx.pid \
#         --lock-path=/var/lock/nginx.lock \
#         --with-http_ssl_module \
#         --with-pcre \
#     && make \
#     && make install

# Install wrk benchmarking tool
# RUN git clone https://github.com/wg/wrk.git /wrk \
#     && cd /wrk \
#     && make -j $NUM_CPU_CORES \
#     && cp wrk /usr/local/bin/ \
#     && rm -rf /wrk


# Install trace-cmd
RUN apt-get install -y trace-cmd

# Download and build libmemcached
# RUN wget https://github.com/awesomized/libmemcached/archive/refs/heads/v1.x.zip -P /memcached && \
#     unzip /memcached/v1.x.zip -d /memcached/extract && \
#     mkdir -p /memcached/build && \
#     cd /memcached/build && \
#     cmake -DENABLE_MEMASLAP=ON ../extract/libmemcached-1.x && \
#     make && make install && \
#     rm -rf /memcached


# Install memtier_benchmark
# RUN mkdir -p /downloads_memtier \
#     && cd /downloads_memtier \
#     && git clone https://github.com/RedisLabs/memtier_benchmark.git \
#     && cd memtier_benchmark \
#     && autoreconf -ivf \
#     && ./configure --prefix=/usr/local \
#     && make \
#     && make install \
#     && cp /usr/local/bin/memtier_benchmark /usr/bin/memtier_benchmark \
#     && rm -rf /downloads_memtier


# Download and extract Geekbench
# RUN wget https://cdn.geekbench.com/Geekbench-6.3.0-Linux.tar.gz && \
#     tar -xzf Geekbench-6.3.0-Linux.tar.gz && \
#     cd Geekbench-6.3.0-Linux && \
#     chmod +x geekbench6 && \
#     cp geekbench6 /usr/bin/ && \
#     cd .. && \
#     rm -rf Geekbench-6.3.0-Linux Geekbench-6.3.0-Linux.tar.gz


# RUN git clone https://github.com/seclab-stonybrook/eaudit.git /root/eaudit

# Create a Python virtual environment and install packages
# RUN python3 -m venv /opt/venv && \
#     /opt/venv/bin/pip install --no-cache-dir matplotlib numpy locust


# Expose necessary ports
# EXPOSE 11211 80 6379 8089

# Start services (Redis and NGINX)
# CMD ["sh", "-c", "nginx -g 'daemon off;'"]