/**
 * User program for loading a single generic program and attaching
 * Usage: ./load_test.user bpf_file 
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>

#include <linux/filter.h>
#include <bpf/bpf.h>
#include <bpf/libbpf.h>

// FYR: if you want to modify topts. check LIBBPF_OPTS definition
// for more details
//LIBBPF_OPTS(bpf_test_run_opts, topts,
//		.data_in = &pkt_v4,
//		.data_size_in = sizeof(pkt_v4),
//		.data_out = buf,
//		.data_size_out = sizeof(buf),
//		.repeat = 1,
//	);

// taken from /tools/testing/selftests/bpf/testing_helpers.c
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
int extra_prog_load_log_flags = 0;

int testing_prog_flags(void)
{
	static int cached_flags = -1;
	static int prog_flags[] = { BPF_F_TEST_RND_HI32, BPF_F_TEST_REG_INVARIANTS };
	static struct bpf_insn insns[] = {
		BPF_MOV64_IMM(BPF_REG_0, 0),
		BPF_EXIT_INSN(),
	};
	int insn_cnt = ARRAY_SIZE(insns), i, fd, flags = 0;
	LIBBPF_OPTS(bpf_prog_load_opts, opts);

	if (cached_flags >= 0)
		return cached_flags;

	for (i = 0; i < ARRAY_SIZE(prog_flags); i++) {
		opts.prog_flags = prog_flags[i];
		fd = bpf_prog_load(BPF_PROG_TYPE_SOCKET_FILTER, "flag-test", "GPL",
				   insns, insn_cnt, &opts);
		if (fd >= 0) {
			flags |= prog_flags[i];
			close(fd);
		}
	}

	cached_flags = flags;
	return cached_flags;
}

int bpf_prog_test_load(const char *file, enum bpf_prog_type type,
		       struct bpf_object **pobj, int *prog_fd)
{
	LIBBPF_OPTS(bpf_object_open_opts, opts,
		.kernel_log_level = extra_prog_load_log_flags,
	);
	struct bpf_object *obj;
	struct bpf_program *prog;
	__u32 flags;
	int err;

	obj = bpf_object__open_file(file, &opts);
	if (!obj)
		return -errno;

	prog = bpf_object__next_program(obj, NULL);
	if (!prog) {
		err = -ENOENT;
		goto err_out;
	}

	if (type != BPF_PROG_TYPE_UNSPEC && bpf_program__type(prog) != type)
		bpf_program__set_type(prog, type);

	flags = bpf_program__flags(prog) | testing_prog_flags();
	bpf_program__set_flags(prog, flags);

	err = bpf_object__load(obj);
	if (err)
		goto err_out;

	*pobj = obj;
	*prog_fd = bpf_program__fd(prog);

	return 0;
err_out:
	bpf_object__close(obj);
	return err;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Not enough args\n");
        printf("Expected: ./load_test.user bpf_file \n");
        return -1;
    }

    char * bpf_path = argv[1];

    struct bpf_object *obj;
    int err, prog_fd;
    
    // void ctxt
    LIBBPF_OPTS(bpf_test_run_opts, topts);

    err = bpf_prog_test_load(bpf_path, BPF_PROG_TYPE_XDP, &obj, &prog_fd);
	if (err < 0)
		return -1;

    err = bpf_prog_test_run_opts(prog_fd, &topts);

    bpf_object__close(obj);

    return 0;
}
