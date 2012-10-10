include ../Make.common

# Sources
cxx_sources :=

c_sources :=    main.c vm.c sched.c

headers_pub :=  sol.h common.h debug.h stdint.h \
                instr.h vm.h sched.h task.h

# --- conf ---------------------------------------------------------------------

project_id = sol

# Source files to Object files
object_dir = $(OBJECT_DIR)-$(project_id)
objects = $(call SrcToObjects,$(object_dir),${cxx_sources:.cc=.o} ${c_sources:.c=.o})
object_dirs = $(call FileDirs,$(objects))
-include ${objects:.o=.d}  # header dependencies

# Public headers
headers_pub_dir = $(INCLUDE_BUILD_PREFIX)/$(project_id)
headers_pub_export = $(call PubHeaderNames,$(headers_pub_dir),$(headers_pub))
headers_pub_export_dirs = $(call FileDirs,$(headers_pub_export))

# Compiler and linker flags
c_flags := $(CFLAGS)
cxx_flags := $(CXXFLAGS)
ld_flags := $(LDFLAGS)
xxld_flags := $(XXLDFLAGS)

# --- targets ---------------------------------------------------------------------

all: $(project_id)

clean:
	rm -rf $(object_dir)
	rm -rf $(headers_pub_export)
#	rm -rf $(LIB_BUILD_PREFIX)/lib$(project_id).a

common_prebuild:
	@mkdir -p $(LIB_BUILD_PREFIX)
	@mkdir -p $(object_dirs)
	@mkdir -p $(headers_pub_export_dirs)

$(project_id): common_prebuild $(headers_pub_export)
#$(LIB_BUILD_PREFIX)/lib$(project_id).a

# Create archive
#$(LIB_BUILD_PREFIX)/lib$(project_id).a: $(objects)
#	$(AR) -rcL $@ $^

# Create program
$(project_id): common_prebuild $(BIN_BUILD_PREFIX)/$(project_id)
#$(BIN_BUILD_PREFIX)/$(project_id): $(LIB_BUILD_PREFIX)/lib$(project_id).a
$(BIN_BUILD_PREFIX)/$(project_id): $(objects)
	$(LD) $(ld_flags) -o $@ $^
#	$(LD) $(xxld_flags) $(ld_flags) -o $@ $^

# C++ source -> object
$(object_dir)/%.o: %.cc
	$(CXXC) $(c_flags) $(cxx_flags) -c -o $@ $<

# C source -> object
$(object_dir)/%.o: %.c
	$(CC) $(c_flags) -c -o $@ $<

# Copy headers into $headers_pub_dir
$(headers_pub_dir)/%: %
	@cp $^ $@

.PHONY: all clean common_prebuild $(project_id)
