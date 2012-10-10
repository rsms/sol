# Compiles all files matching "test_*.{c,cc}" in this directory and runs the
# resulting programs. Will call make in dependencies when needed.
#
override DEBUG = 1 # Always build tests in debug mode
include ../Make.common

bin_prefix := $(BIN_BUILD_PREFIX)/test
object_dir = $(OBJECT_DIR)-test

# Compiler and linker flags
c_flags := $(CFLAGS)
cxx_flags := $(CXXFLAGS)
ld_flags := $(LDFLAGS)
xxld_flags := $(XXLDFLAGS)

c_test_sources := $(wildcard test_*.c)
c_test_objects  = $(patsubst %.c,$(object_dir)/%.o,$(c_test_sources))
c_test_programs = $(patsubst %.c,$(bin_prefix)/%.ctest,$(c_test_sources))

# --- targets ---

all: test

test: common_prebuild
test: $(c_test_programs)
#test:
#	@echo c_test_sources = $(c_test_sources)
#	@echo c_test_objects = $(c_test_objects)
#	@echo c_test_programs = $(c_test_programs)

-include ${c_test_objects:.o=.d}

# Link and run C tests
$(bin_prefix)/%.ctest: $(object_dir)/%.o
	$(LD) $(ld_flags) -o $@ $^
	@echo -n Running test: $(patsubst %.ctest,%.c,$(@F)) ... 
	@$@
	@echo PASS

# Link and run C++ tests
$(bin_prefix)/%.cxxtest: $(object_dir)/%.o
	$(LD) $(xxld_flags) $(ld_flags) -o $@ $^
	@echo -n Running test: $(patsubst %.cxxtest,%.cc,$(@F)) ...
	@$@
	@echo PASS

# C++ source -> object
$(object_dir)/%.o: %.cc
	$(CXXC) $(c_flags) $(cxx_flags) -c -o $@ $<

# C source -> object
$(object_dir)/%.o: %.c
	$(CC) $(c_flags) -c -o $@ $<

clean:
	rm -rf $(object_dir)
	rm -rf $(bin_prefix)

common_prebuild:
	@$(MAKE) -C $(SRCROOT)/sol
	@mkdir -p $(bin_prefix)
	@mkdir -p $(object_dir)

.PHONY: all clean common_prebuild test $(bin_prefix)/%.ctest
