#include "vm.h"
#include "instr.h"

// Toggle to enable debug logging (activates `SVMDLog` macros.)
#define S_VM_DEBUG_LOG 1

// Execution limit -- limits the number of instructions that can be executed by
// a task in one scheduled run. Set to 0 to disable limiting. Since tasks are
// cooperatively multitasking, one task might hog a scheduler by executing a
// large number of instructions without yielding. If a limit is set, then such
// a task will be rescheduled (forced to yield), allowing other tasks to execute
// some code.
#define S_VM_EXEC_LIMIT 2

#include "vm_internal_debug.h"

// Inspect generated code:
// clang -I. -O2 -S -o sol/vm.s sol/vm.c
// clang -I. -O2 -S -emit-llvm -o sol/vm.ll sol/vm.c

int sticky; // XXX DEBUG "tricking" the compiler not to optimize away branches

STaskStatus SVMExec(STask *task) {

  // Local PC which we store back into `task` when returning
  SInstr *pc = task->pc;

  #if S_VM_EXEC_LIMIT
  // Number of instructions executed. We need to keep a dedicated counter since
  // JUMPs offset `pc` and so there's no way of telling how many instructions
  // have been executed from comparing `pc` to for instance `task->pc`.
  size_t icounter = 0;
  #endif // S_VM_EXEC_LIMIT

  while (1) {
    switch (SInstrGetOP(*++pc)) {
    case S_OP_RETURN: {
      SVMDLogOpAB();
      task->pc = task->start;
      return STaskStatusEnd;
    }
    case S_OP_YIELD: {
      SVMDLogOpAB();
      task->pc = pc;
      return STaskStatusYield;
    }
    case S_OP_MOVE: {
      SVMDLogOpAB();
      sticky = S_OP_MOVE+100;
      break;
    }
    case S_OP_LOADK: {
      SVMDLogOpABu();
      sticky = S_OP_LOADK+100;
      break;
    }
    case S_OP_ADDI: {
      SVMDLogOpABC();
      sticky = S_OP_ADDI+100;
      break;
    }
    case S_OP_SUBI: {
      SVMDLogOpABC();
      sticky = S_OP_SUBI+100;
      break;
    }
    case S_OP_MULI: {
      SVMDLogOpABC();
      sticky = S_OP_MULI+100;
      break;
    }
    case S_OP_DIVI: {
      SVMDLogOpABC();
      sticky = S_OP_DIVI+100;
      break;
    }
    case S_OP_NOT: {
      SVMDLogOpAB();
      sticky = S_OP_NOT+100;
      break;
    }
    case S_OP_EQ: {
      SVMDLogOpABC();
      sticky = S_OP_EQ+100;
      break;
    }
    case S_OP_LT: {
      SVMDLogOpABC();
      sticky = S_OP_LT+100;
      break;
    }
    case S_OP_LE: {
      SVMDLogOpABC();
      sticky = S_OP_LE+100;
      break;
    }
    case S_OP_JUMP: {
      SVMDLogOpBss();
      sticky = S_OP_JUMP+100;
      break;
    }
    default:
      SVMDLogOp("unexpected operation");
      S_UNREACHABLE;
    }

    #if S_VM_EXEC_LIMIT
    // Reached execution limit?
    if (++icounter >= S_VM_EXEC_LIMIT) {
      SVMDLog("execution limit (" S_STR(S_VM_EXEC_LIMIT) ") reached -- yielding");
      task->pc = pc;
      return STaskStatusYield;
    }
    #endif
  } // while (1)

  S_UNREACHABLE;
  return STaskStatusError;
}