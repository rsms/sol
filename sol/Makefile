include ../Make.common

# Sources
cxx_sources :=

c_sources :=    log.c host.c msg.c \
                sched.c task.c func.c \
                value.c

headers_pub :=  sol.h common.h common_target.h common_stdint.h common_atomic.h \
                debug.h log.h host.h msg.h \
                vm.h sched.h runq.h task.h func.h arec.h instr.h \
                value.h

main_c_sources := main.c

# --- conf ---------------------------------------------------------------------

project_id = sol
all: $(project_id)

# Source files to Object files
object_dir = $(OBJECT_DIR)-$(project_id)
objects = $(call SrcToObjects,$(object_dir),${cxx_sources:.cc=.o} ${c_sources:.c=.o})
main_objects = $(call SrcToObjects,$(object_dir),${main_c_sources:.c=.o})
object_dirs = $(call FileDirs,$(objects)) $(call FileDirs,$(main_objects))

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

# Sol library and program
static_library = $(LIB_BUILD_PREFIX)/lib$(project_id).a
main_program = $(BIN_BUILD_PREFIX)/$(project_id)

# Compiler and linker flags
c_flags := $(CFLAGS) -MMD
cxx_flags := $(CXXFLAGS)
ld_flags := $(LDFLAGS) -L"$(LIB_BUILD_PREFIX)"
xxld_flags := $(XXLDFLAGS)

# --- targets ---------------------------------------------------------------------

clean:
	rm -rf $(object_dir)
	rm -rf $(headers_pub_export)
	rm -rf $(asmout_dir)
	rm -f $(main_program)
	rm -f $(static_library)
	rm -f "$(LIB_BUILD_PREFIX)"/libev.a
	@$(MAKE) -C "$(SRCROOT)"/deps/libev clean

common_pre:
	@mkdir -p $(object_dirs)
	@mkdir -p $(headers_pub_export_dirs)

# Create program
$(project_id): common_pre lib$(project_id) libev $(main_program)
$(main_program): $(main_objects)
	@mkdir -p $(dir $(main_program))
	$(LD) $(ld_flags) -lev -l$(project_id) -o $@ $^

# External dependency: libev
libev:
	@$(MAKE) -C $(SRCROOT)/deps/libev
	@mkdir -p "$(LIB_BUILD_PREFIX)"
	@ln -fs "$(SRCROOT)/deps/libev/.libs/libev.a" "$(LIB_BUILD_PREFIX)"

# Create library archive
lib$(project_id): common_pre $(headers_pub_export) $(static_library)
$(static_library): $(objects)
	@mkdir -p $(dir $(static_library))
	$(AR) -rc $@ $^

# Generate LLVM IS code (.ll files)
llvm_ir_pre: common_pre
	@mkdir -p $(asmout_ll_dirs)
llvm_ir: llvm_ir_pre $(asmout_ll)

# Generate target assembly code (.s files)
asm_pre: common_pre
	@mkdir -p $(asmout_s_dirs)
asm: asm_pre $(asmout_s)

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

# header dependencies
-include ${objects:.o=.d}
-include ${main_objects:.o=.d}

.PHONY: all clean common_pre $(project_id) libev lib$(project_id) \
	      llvm_ir_pre asm_pre
