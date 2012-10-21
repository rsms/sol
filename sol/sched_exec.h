#line 1 "sched_exec.h"
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
  #define S_VM_EXEC_LIMIT_INCR(increment) (icounter += (increment))
#else
  #define S_VM_EXEC_LIMIT_INCR(x) ((void)0)
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
RK_(uint32_t index, SValue* constants, SValue* registry) {
  return (index < S_INSTR_RK_k) ? registry[index]
                                : constants[index - S_INSTR_RK_k];
}

inline static STaskStatus S_ALWAYS_INLINE
SSchedExec(SVM* vm, SSched* sched, STask *task) {

  // Get current activation record and set `pc` to the PC of that AR
  SARec *ar = task->ar;
  SInstr *pc = ar->pc;

  // Local pointer to the activation record's constants and registry
  SValue* constants = ar->func->constants;
  SValue* registry = ar->registry;

  // Helpers for accessing constants and registers
  #define K_B(i)   (constants[SInstrGetB(i)])
  #define K_Bu(i)  (constants[SInstrGetBu(i)])
  #define K_C(i)   (constants[SInstrGetC(i)])
  #define R_A(i)   (registry[SInstrGetA(i)])
  #define R_B(i)   (registry[SInstrGetB(i)])
  #define R_C(i)   (registry[SInstrGetC(i)])
  #define RK_B(i)  RK_(SInstrGetB(i), constants, registry)
  #define RK_C(i)  RK_(SInstrGetC(i), constants, registry)
  #define RK_Bu(i) RK_(SInstrGetBu(i), constants, registry)

  #if S_VM_EXEC_LIMIT
  // Number of instructions executed. We need to keep a dedicated counter since
  // JUMPs offset `pc` and so there's no way of telling how many instructions
  // have been executed from comparing `pc` to for instance `task->pc`.
  // Most operations have a cost of 1, but some operations, like CALL with
  // arguments, might have a higher cost in which case that instruction's logic
  // increments this counter in addition to the monotonic increment per op.
  // The S_VM_EXEC_LIMIT_INCR macro is used for this purpose.
  size_t icounter = 0;
  #endif // S_VM_EXEC_LIMIT

  while (1) {
    switch (SInstrGetOP(*++pc)) {

    case S_OP_YIELD: {
      // YIELD A=<type> ...
      // YIELD A=0 -- Yield for other tasks (reschedule)
      // YIELD A=1 B=<rk afterv> -- Wait for timeout
      SVMDLogOpABC();
      ar->pc = pc;
      switch (SInstrGetA(*pc)) {
      case 0: {
        return STaskStatusYield;
      }
      case 1: {
        // TODO The task is waiting for a timeout. The task wants to be resumed
        // after RK(B) = after_ms elapsed.
        assert(RK_B(*pc).type == SValueTNumber);
        SNumber after_ms = RK_B(*pc).value.n;
        SSchedTimerStart(sched, task, after_ms, (SNumber)0);

        return STaskStatusSuspend;
      }
      default: {
        SVMDLogOp("unexpected yield type %u", SInstrGetA(*pc));
        return STaskStatusError;
      }
      }
    }

    case S_OP_JUMP: {
      SVMDLogOpBss();
      pc += SInstrGetBss(*pc);
      break;
    }

    case S_OP_CALL: {
      // CALL A B C -> R(A), ... ,R(A+C-1) := R(A)(R(A+1), ... ,R(A+B))
      //               Start, ... Length  =  fun( Start, ...  Length )
      // Examples:
      //   CALL 3 0 0 = 3(4..3) = 3()        -> 3..2 -> <nothing>
      //   CALL 3 3 1 = 3(4..6) = 3(4, 5, 6) -> 3..3 -> <one return value>
      //   CALL 3 1 3 = 3(4..4) = 3(4)       -> 3..5 -> <three return values>
      SVMDLogOpABC();
      assert(R_A(*pc).type == SValueTFunc);
      
      // Keep a temporary reference to the current activation record and store
      // the current PC back into the AR
      SARec* parent_ar = ar;
      ar->pc = pc;

      // Create a new activation record
      ar = SARecCreate((SFunc*)R_A(*pc).value.p, parent_ar);

      // Copy any arguments into the new AR's registry
      uint16_t argc = SInstrGetB(*pc);
      if (argc != 0) {
        // Add +1 to execution cost
        S_VM_EXEC_LIMIT_INCR(1);

        // copy &reg[A+1],len from parent reg to new reg
        memcpy(
          (void*)ar->registry,
          (const void*)&parent_ar->registry[SInstrGetA(*pc)+1],
          sizeof(SValue) * argc
        );
      }

      // Push the new AR to the top of the task's AR stack
      task->ar = ar;

      // Update our references
      pc = ar->pc;
      constants = ar->func->constants;
      registry = ar->registry;

      break;
    } // case S_OP_CALL

    case S_OP_RETURN: {
      // return R(A), ... ,R(A+B-1)
      SVMDLogOpAB();

      if (ar->parent == 0) {
        // This is the last activation record -- entry function. So let's exit
        // the task.
        ar->pc = pc;
        return STaskStatusEnd;

      } else {
        // Keep a temporary reference to the returning-from AR
        SARec* prev_ar = ar;
        SInstr* prev_pc = pc;

        // Pop the activation record for the returning-from closure from the
        // task's AR stack
        ar = prev_ar->parent;
        task->ar = ar;

        // Update our references
        pc = ar->pc;
        constants = ar->func->constants;
        registry = ar->registry;

        // Add +1 to execution cost
        S_VM_EXEC_LIMIT_INCR(1);

        // Copy any return values into the new AR's registry
        uint16_t resc = SInstrGetB(*pc);
        if (resc != 0) {
          // Since the CALL instruction decides where return arguments go into
          // the callers registry, we must get the landing offset and length
          // from the call instruction.
          assert(SInstrGetOP(*pc) == S_OP_CALL);
          // CALL A B C -> R(A), ... ,R(A+C-1) := R(A)(R(A+1), ... ,R(A+B))
          //               Start, ... Length  =  fun( Start, ...  Length )
          uint16_t dstlen = SInstrGetC(*pc);
          if (dstlen != 0) {
            uint8_t srcri = SInstrGetA(*prev_pc);
            uint8_t dstri = SInstrGetA(*pc);
            //SLogD("resc: %u, dstlen: %u, dstri: %u", resc, dstlen, dstri);
            memcpy(
              (void*)&ar->registry[dstri],
              (const void*)&prev_ar->registry[srcri],
              sizeof(SValue) * (resc < dstlen ? resc : dstlen)
            );
          }
        } // end: copying return values.

        // Discard returned-from ativation record
        SARecDestroy(prev_ar);
        break;
      }
    } // case S_OP_RETURN

    case S_OP_SPAWN: {  // R(A) = spawn(RK(B))
      SVMDLogOpAB();
      assert(RK_B(*pc).type == SValueTFunc);
      SFunc* func = (SFunc*)RK_B(*pc).value.p;
      STask* t = STaskCreate(func, task, 0);
      STaskRetain(task);
      _RQPush(sched, t);
      SLogD("[task %p] spawned new [task %p]", task, t);
      break;
    }

    case S_OP_LOADK: {  // R(A) = K(Bu)
      SVMDLogOpABu();
      R_A(*pc) = K_Bu(*pc);
      break;
    }

    case S_OP_MOVE: {  // R(A) = R(B)
      SVMDLogOpAB();
      R_A(*pc) = R_B(*pc);
      break;
    }

    case S_OP_DBGREG: { // special debug: dump ABC register values
      SVMDLogOp();
      SVMDLogInstrRVal(A, *pc);
      SVMDLogInstrRVal(B, *pc);
      SVMDLogInstrRVal(C, *pc);
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
      R_A(*pc).value.n = RK_B(*pc).value.n * RK_C(*pc).value.n;
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

    default:
      SVMDLogOp("unexpected operation");
      return STaskStatusError;
    }

    #if S_VM_EXEC_LIMIT
    // Reached execution limit?
    if (++icounter >= S_VM_EXEC_LIMIT) {
      SVMDLog("execution limit (" S_STR(S_VM_EXEC_LIMIT)
              ") reached -- yielding");
      ar->pc = pc;
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
