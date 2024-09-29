#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>
#include <stdlib.h>

#include <linux/bpf.h>
#include <bpf/bpf.h>
#include <bpf/libbpf.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Not enough args\n");
        printf("Expected: ./load.user bpf_file bpf_prog_name\n");
        return -1;
    }

    char * bpf_path = argv[1];
    char * prog_name = argv[2];

    int stats_fd = bpf_enable_stats(BPF_STATS_RUN_TIME);
    if (stats_fd < 0) {
        printf("Failed to enable the stats with err %d\n", stats_fd);
	return 0;
    }
    struct bpf_prog_info info;
    __u32 info_len = sizeof(info);

    struct bpf_object * prog = bpf_object__open(bpf_path);
    
    if (bpf_object__load(prog)) {
        printf("Failed to load(verify)\n");
        return 0;
    }

    struct bpf_program * program = bpf_object__find_program_by_name(prog, prog_name);
    int prog_fd = bpf_program__fd(program);
    if (program == NULL) {
        printf("Failed to find the program\n");
        return 0;
    }

    struct bpf_link *link = bpf_program__attach(program);
    if (libbpf_get_error(link)) {
        printf("Attachment failed with error %ld\n", libbpf_get_error(link));
        return 0;
    }

    printf("Attachment id done\n");

    printf("Collecting the data every second\n");
    printf("runtime(ns)\trun_cnt\n");
    __u64 prev_run_time_ns = 0;
    __u64 prev_run_cnt = 0;
    while (1) {
	
    	memset(&info, 0, info_len);
	int err = bpf_prog_get_info_by_fd(prog_fd, &info, &info_len);
	if (err < 0) {
		printf("Failed to fetch bpf prog info with err %d\n", err);
		return 0;
	}
	__u64 curr_run_cnt = info.run_cnt - prev_run_cnt;
	__u64 per_run_time_ns = (info.run_time_ns - prev_run_time_ns) / curr_run_cnt;
	printf("%lld\t%lld\n", per_run_time_ns, curr_run_cnt);
	prev_run_time_ns = info.run_time_ns;
	prev_run_cnt = info.run_cnt;
	sleep(1);
    }

    return 0;
}
