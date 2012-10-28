# Compiles all files matching "test_*.{c,cc}" in this directory and runs the
# resulting programs. Will call make in dependencies when needed.
#
override DEBUG = 1 # Always build tests in debug mode
include ../Make.common

bin_prefix := $(TESTS_BUILD_PREFIX)
object_dir = $(TESTS_BUILD_PREFIX)/.objs

# Compiler and linker flags
c_flags := $(CFLAGS) -I.. -DS_TEST_SUIT_RUNNING=1
cxx_flags := $(CXXFLAGS)
ld_flags := $(LDFLAGS) -L"$(LIB_BUILD_PREFIX)"
xxld_flags := $(XXLDFLAGS)

# TODO: Make it possible to disable SMP tests, so that we can run there tests on
#       UP systems.
c_test_sources  := $(wildcard test_*.c)
c_test_programs  = $(patsubst %.c,$(bin_prefix)/%.c-up,$(c_test_sources))
c_test_programs += $(patsubst %.c,$(bin_prefix)/%.c-smp,$(c_test_sources))
c_test_programs := $(sort $(c_test_programs))

# --- targets ---

all: test

test: common_pre
test: $(c_test_programs)
#test:
#	@echo c_test_sources = $(c_test_sources)
#	@echo c_test_objects = $(c_test_objects)
#	@echo c_test_programs = $(c_test_programs)

-include ${c_test_sources:.c=.d}

clean:
	rm -rf $(object_dir)
	rm -rf $(bin_prefix)

common_pre:
	@$(MAKE) -C $(SRCROOT)/sol DEBUG=1 sol
	@mkdir -p $(bin_prefix)
	@mkdir -p $(object_dir)

# Link and run tests (UP)
$(bin_prefix)/%.c-up: $(object_dir)/%.up.o
	$(LD) $(ld_flags) -lev -lsol -o $@ $^
	@printf "Running test: %s (UP) ... " $(patsubst %.c-up,%.c,$(@F))
	@$@ >/dev/null
	@echo PASS
	@rm -f $@ $(object_dir)/%.up.o

# Link and run tests (SMP)
$(bin_prefix)/%.c-smp: $(object_dir)/%.smp.o
	$(LD) $(ld_flags) -lev -lsol -lpthread -o $@ $^
	@printf "Running test: %s (SMP) ... " $(patsubst %.c-smp,%.c,$(@F))
	@$@ >/dev/null
	@echo PASS
	@rm -f $@ $(object_dir)/%.smp.o

# C source -> object
$(object_dir)/%.up.o: %.c
	$(CC) $(c_flags) -MMD -DS_WITHOUT_SMP=1 -c -o $@ $<
$(object_dir)/%.smp.o: %.c
	$(CC) $(c_flags) -MMD -c -o $@ $<

.PHONY: all clean common_pre test $(bin_prefix)/%.c.up \
	      $(bin_prefix)/%.c.mp test_%.c
