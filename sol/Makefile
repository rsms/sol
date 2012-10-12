include ../Make.common

# Sources
cxx_sources :=

c_sources :=    main.c \
                log.c host.c \
                sched.c task.c \
                value.c

headers_pub :=  sol.h \
                common.h common_target.h common_stdint.h \
                debug.h log.h host.h \
                vm.h sched.h runq.h task.h instr.h \
                value.h

# --- conf ---------------------------------------------------------------------

project_id = sol

# Source files to Object files
object_dir = $(OBJECT_DIR)-$(project_id)
objects = $(call SrcToObjects,$(object_dir),${cxx_sources:.cc=.o} ${c_sources:.c=.o})
object_dirs = $(call FileDirs,$(objects))
-include ${objects:.o=.d}  # header dependencies

# For LLVM IR and Assembly output
asmout_dir = $(DEBUG_BUILD_PREFIX)/$(project_id)-asm
asmout_ll = $(call SrcToObjects,$(asmout_dir),${cxx_sources:.cc=.ll} ${c_sources:.c=.ll})
asmout_ll_dirs = $(call FileDirs,$(asmout_ll))
asmout_s = $(call SrcToObjects,$(asmout_dir),${cxx_sources:.cc=.s} ${c_sources:.c=.s})
asmout_s_dirs = $(call FileDirs,$(asmout_s))

# Public headers
headers_pub_dir = $(INCLUDE_BUILD_PREFIX)/$(project_id)
headers_pub_export = $(call PubHeaderNames,$(headers_pub_dir),$(headers_pub))
headers_pub_export_dirs = $(call FileDirs,$(headers_pub_export))

# Sol program
main_program = $(BIN_BUILD_PREFIX)/$(project_id)

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
	rm -rf $(asmout_dir)
	rm -f $(main_program)
#	rm -rf $(LIB_BUILD_PREFIX)/lib$(project_id).a

common_prebuild:
	@mkdir -p $(dir $(main_program))
	@mkdir -p $(object_dirs)
	@mkdir -p $(headers_pub_export_dirs)
#	@mkdir -p $(LIB_BUILD_PREFIX)

$(project_id): common_prebuild $(headers_pub_export)
#$(LIB_BUILD_PREFIX)/lib$(project_id).a

# Create archive
#$(LIB_BUILD_PREFIX)/lib$(project_id).a: $(objects)
#	$(AR) -rcL $@ $^

# Create program
$(project_id): common_prebuild $(main_program)
#$(main_program): $(LIB_BUILD_PREFIX)/lib$(project_id).a
$(main_program): $(objects)
	$(LD) $(ld_flags) -o $@ $^
#	$(LD) $(xxld_flags) $(ld_flags) -o $@ $^

# Generate LLVM IS code (.ll files)
llvm_ir_prebuild: common_prebuild
	@mkdir -p $(asmout_ll_dirs)
llvm_ir: llvm_ir_prebuild $(asmout_ll)

# Generate target assembly code (.s files)
asm_prebuild: common_prebuild
	@mkdir -p $(asmout_s_dirs)
asm: asm_prebuild $(asmout_s)

# C++ source -> object
$(object_dir)/%.o: %.cc
	$(CXXC) $(c_flags) $(cxx_flags) -c -o $@ $<

# C source -> object
$(object_dir)/%.o: %.c
	$(CC) $(c_flags) -c -o $@ $<

# C source -> LLVM IR
$(asmout_dir)/%.ll: %.c
	clang $(CFLAGS) -S -emit-llvm  -DS_CODEGEN_ASM=1 -DS_CODEGEN_LLVMIR=1 -o $@ $<

# C source -> target assembly
$(asmout_dir)/%.s: %.c
	$(CC) $(CFLAGS) -S -DS_CODEGEN_ASM=1 -o $@ $<

# Copy headers into $headers_pub_dir
$(headers_pub_dir)/%: %
	@cp $^ $@

.PHONY: all clean common_prebuild $(project_id)
