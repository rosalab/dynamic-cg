SOURCES := $(wildcard *.kern.c)
FILES := $(SOURCES:.c=.o)
USER_SRC := $(wildcard *.user.c)
USER := $(USER_SRC:.c=)

BPF-CLANG := clang
BPF_CLANG_CFLAGS := -target bpf -g -Wall -O2 -c
INCLUDE := -I../../../../../usr/include/ -I../../../../../tools/lib/
USER-CFLAGS := -g -I../../../../../tools/include/ -I../../../../../usr/include -I../../../../../tools/lib/ -L../../../../../tools/lib/bpf/

all: $(FILES) $(USER)
	
test: test.c
	$(BPF-CLANG) $(INCLUDE) $(BPF_CLANG_CFLAGS) -o test test.c

$(FILES) : %.o : %.c
	$(BPF-CLANG) $(INCLUDE) $(BPF_CLANG_CFLAGS) -o $@ $<

$(USER) : % : %.c
	gcc $(USER-CFLAGS) $< -lbpf -o $@

.PHONY : clean

clean :
	rm $(FILES) $(USER)
