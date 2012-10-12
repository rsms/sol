// Task program execution.
//
// Note: This file is internal and part of sched.c
//
// Note: There are some mixed names in here (SSched and SVM) since logically
// this code is both part of the VM and the scheduler.
//
#include "task.h"
#include "common.h"
#include "vm.h"
#include "log.h"

// Toggle to enable debug logging (activates `SVMDLog` macros.)
#ifndef S_VM_DEBUG_LOG
#define S_VM_DEBUG_LOG S_DEBUG
#endif

// Execution limit -- limits the number of instructions that can be executed by
// a task in one scheduled run. Set to 0 to disable limiting. Since tasks are
// cooperatively multitasking, one task might hog a scheduler by executing a
// large number of instructions without yielding. If a limit is set, then such
// a task will be rescheduled (forced to yield), allowing other tasks to execute
// some code.
#ifndef S_VM_EXEC_LIMIT
#define S_VM_EXEC_LIMIT 100
#endif

// Contains SVMDLog macros
#include "sched_exec_debug.h"

// Inspect generated code by:
//   make -C ./sol llvm_ir asm && $EDITOR build/debug/sol-asm/sched.{ll,s}
// Or:
//   clang -I. -O2 -std=c99 -S -o - sol/sched.c | $EDITOR
//   clang -I. -O2 -std=c99 -S -emit-llvm -o - sol/sched.c | $EDITOR
//

// RK_(index)
inline static SValue S_ALWAYS_INLINE
RK_(uint16_t index, SValue* constants, SValue* registry) {
  return (index < 255) ? registry[index] : constants[index - 255];
}

inline static STaskStatus S_ALWAYS_INLINE
SSchedExec(SVM* vm, SSched* sched, STask *task) {

  // Local PC which we store back into `task` when returning
  SInstr *pc = task->pc;
  if (pc == task->start) {
    // Initial execution will cause the PC to move one step ahead, so we rewind
    --pc;
  }

  // Local pointer to task's constants and registry
  SValue* constants = task->constants;
  SValue* registry = task->registry;

  // Helpers for accessing constants and registers
  #define K_B(i) (constants[SInstrGetB(i)])
  #define K_C(i) (constants[SInstrGetC(i)])
  #define R_A(i) (registry[SInstrGetA(i)])
  #define R_B(i) (registry[SInstrGetB(i)])
  #define R_C(i) (registry[SInstrGetC(i)])
  #define RK_B(i) RK_(SInstrGetB(i), constants, registry)
  #define RK_C(i) RK_(SInstrGetC(i), constants, registry)

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
      SVMDLogOpABu();
      task->pc = pc;
      switch (SInstrGetA(*pc)) {
      case 1: {
        // TODO: SInstrGetBu() <- FD the task is waiting for
        return STaskStatusWait;
      }
      case 2: {
        // TODO: SInstrGetBu() <- timer the task is waiting for
        return STaskStatusWait;
      }
      default: {
        return STaskStatusYield;
      }
      }
    }

    case S_OP_LOADK: {  // R(A) = K(Bu)
      SVMDLogOpAB();
      R_A(*pc) = K_B(*pc);
      //SVMDLogInstrRVal(A, *pc);
      break;
    }

    case S_OP_MOVE: {  // R(A) = R(B)
      SVMDLogOpAB();
      R_A(*pc) = R_B(*pc);
      break;
    }

    case S_OP_ADD: { // R(A) = RK(B) + RK(C)
      SVMDLogOpABC();
      R_A(*pc).value.n = RK_B(*pc).value.n + RK_C(*pc).value.n;
      break;
    }

    case S_OP_SUB: { // R(A) = RK(B) - RK(C)
      SVMDLogOpABC();
      // SVMDLogInstrRKVal(B, *pc);
      // SVMDLogInstrRKVal(C, *pc);
      // SVMDLogInstrRVal(A, *pc);
      R_A(*pc).value.n = RK_B(*pc).value.n - RK_C(*pc).value.n;
      break;
    }

    case S_OP_MUL: { // R(A) = RK(B) * RK(C)
      SVMDLogOpABC();
      R_A(*pc).value.n = RK_B(*pc).value.n / RK_C(*pc).value.n;
      break;
    }

    case S_OP_DIV: { // R(A) = RK(B) / RK(C)
      SVMDLogOpABC();
      R_A(*pc).value.n = RK_B(*pc).value.n / RK_C(*pc).value.n;
      break;
    }

    case S_OP_NOT: { // R(A) = not R(B)
      SVMDLogOpAB();
      // TODO: Needs testing
      R_A(*pc).value.n = ! R_B(*pc).value.n;
      break;
    }

    case S_OP_EQ: { // if (RK(B) == RK(C)) JUMP else PC++
      SVMDLogOpABC();
      // TODO: Needs testing
      if (RK_B(*pc).value.n == RK_C(*pc).value.n) {
        ++pc;
        assert(SInstrGetOP(*pc) == S_OP_JUMP);
        SVMDLogOpBss();
        pc += SInstrGetBss(*pc);
      } else {
        ++pc;
      }
      break;
    }

    case S_OP_LT: { // if (RK(B) < RK(C)) JUMP else PC++
      SVMDLogOpABC();
      // TODO: Needs testing
      if (RK_B(*pc).value.n < RK_C(*pc).value.n) {
        ++pc;
        assert(SInstrGetOP(*pc) == S_OP_JUMP);
        SVMDLogOpBss();
        pc += SInstrGetBss(*pc);
      } else {
        ++pc;
      }
      break;
    }

    case S_OP_LE: { // if (RK(B) <= RK(C)) JUMP else PC++
      SVMDLogOpABC();
      //SVMDLogInstrRKVal(B, *pc);
      //SVMDLogInstrRKVal(C, *pc);
      if (RK_B(*pc).value.n <= RK_C(*pc).value.n) {
        // Fetch the upcoming JUMP instruction (always follows a test)
        //SLogD("(B <= B) = true");
        ++pc;
        assert(SInstrGetOP(*pc) == S_OP_JUMP);
        SVMDLogOpBss();
        pc += SInstrGetBss(*pc);
      } else {
        // Failed. Skip the JUMP instruction
        //SLogD("(b <= c) = false");
        ++pc;
      }
      break;
    }

    case S_OP_JUMP: {
      SVMDLogOpBss();
      pc += SInstrGetBss(*pc);
      break;
    }

    default:
      SVMDLogOp("unexpected operation");
      return STaskStatusError;
    }

    #if S_VM_EXEC_LIMIT
    // Reached execution limit?
    if (++icounter >= S_VM_EXEC_LIMIT) {
      SVMDLog("execution limit (" S_STR(S_VM_EXEC_LIMIT)
              ") reached -- yielding");
      task->pc = pc;
      return STaskStatusYield;
    }
    #endif
  } // while (1)

  #undef R_A
  #undef R_B
  #undef R_C

  S_UNREACHABLE;
  return STaskStatusError;
}
