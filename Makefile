c_sources := sol/main.c sol/vm.c sol/sched.c

all: build/bin/sol

build_dir = build
object_dir = $(build_dir)/.objs
objects = $(patsubst %,$(object_dir)/%,${c_sources:.c=.o})
object_dirs = $(sort $(foreach fn,$(objects),$(dir $(fn))))
-include ${objects:.o=.d}

CC = clang
LD = clang

CFLAGS 	+= -Wall -g -MMD -std=c99 -I.
#LDFLAGS +=
ifneq ($(DEBUG),)
# When make is run like with: make DEBUG=1
	CFLAGS += -O0
else
	CFLAGS += -O3 -DNDEBUG
endif

clean:
	@rm -rf $(build_dir)

$(build_dir)/bin/sol: $(objects)
	@mkdir -p `dirname $@`
	$(LD) $(LDFLAGS) -o $@ $^

$(object_dir)/%.o: %.c
	@mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean all
