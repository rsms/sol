# Compiles all files matching "test_*.{c,cc}" in this directory and runs the
# resulting programs. Will call make in dependencies when needed.
#
override DEBUG = 1 # Always build tests in debug mode
include ../Make.common

bin_prefix := $(TESTS_BUILD_PREFIX)
object_dir = $(TESTS_BUILD_PREFIX)/.objs

# Compiler and linker flags
c_flags := $(CFLAGS) -DS_TEST_SUIT_RUNNING=1
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

test: common_prebuild
test: $(c_test_programs)
#test:
#	@echo c_test_sources = $(c_test_sources)
#	@echo c_test_objects = $(c_test_objects)
#	@echo c_test_programs = $(c_test_programs)

-include ${c_test_sources:.c=.d}

# Link and run tests (UP)
$(bin_prefix)/%.c-up: $(object_dir)/%.up.o
	@$(LD) $(ld_flags) -o $@ $^
	@printf "Running test: %s (UP) ... " $(patsubst %.c-up,%.c,$(@F))
	@$@
	@echo PASS
	@rm -f $@ $(object_dir)/%.up.o

# Link and run tests (SMP)
$(bin_prefix)/%.c-smp: $(object_dir)/%.smp.o
	@$(LD) $(ld_flags) -lpthread -o $@ $^
	@printf "Running test: %s (SMP) ... " $(patsubst %.c-smp,%.c,$(@F))
	@$@
	@echo PASS
	@rm -f $@ $(object_dir)/%.smp.o

# C source -> object
$(object_dir)/%.up.o: %.c
	@$(CC) $(c_flags) -MMD -DS_WITHOUT_SMP=1 -c -o $@ $<
$(object_dir)/%.smp.o: %.c
	@$(CC) $(c_flags) -MMD -c -o $@ $<

clean:
	rm -rf $(object_dir)
	rm -rf $(bin_prefix)

common_prebuild:
	@$(MAKE) DEBUG=1 -C $(SRCROOT) libev
	@$(MAKE) DEBUG=1 -C $(SRCROOT)/sol
	@mkdir -p $(bin_prefix)
	@mkdir -p $(object_dir)

.PHONY: all clean common_prebuild test $(bin_prefix)/%.c.up \
	      $(bin_prefix)/%.c.mp
