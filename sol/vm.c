#include "vm.h"
#include "instr.h"

STaskStatus SVMExec(STask *task) {
  static const void const* oct[] = {
    #define OP_TABLE(name, _) &&on__##name,
    S_INSTR_DEFINE(OP_TABLE)
    #undef  OP_TABLE
  };

  static const char const* opnames[] = {
    #define OP_TABLE(name, _) #name,
    S_INSTR_DEFINE(OP_TABLE)
    #undef  OP_TABLE
  };

  // Debug logging macros
  #define SDLogOp(fmt, ...) \
    printf("[vm] %p %-10u [%02u] %-6s" fmt "\n", \
      task, (uint32_t)(pc - task->start), \
      (uint8_t)SInstrGetOP(pc[0]), opnames[SInstrGetOP(pc[0])], \
      ##__VA_ARGS__ \
    )
  #define SDLogOpA SDLogOp(  "  A:     %3u", (uint8_t)SInstrGetA(pc[0]))
  #define SDLogOpAB SDLogOp( "  A,B:   %3u, %3u", \
    (uint8_t)SInstrGetA(pc[0]), (uint16_t)SInstrGetB(pc[0]))
  #define SDLogOpABC SDLogOp("  A,B,C: %3u, %3u, %3u", \
    (uint8_t)SInstrGetA(pc[0]), (uint16_t)SInstrGetB(pc[0]), \
    (uint16_t)SInstrGetC(pc[0]))
  #define SDLogOpABs SDLogOp("  A,Bs:  %3u, %6d", \
    (uint8_t)SInstrGetA(pc[0]), SInstrGetBs(pc[0]))
  #define SDLogOpABu SDLogOp("  A,Bu:  %3u, %6u", \
    (uint8_t)SInstrGetA(pc[0]), SInstrGetBu(pc[0]))
  #define SDLogOpBss SDLogOp("  Bss:   %8d", SInstrGetBss(pc[0]))
  #define SDLogOpBuu SDLogOp("  Buu:   %8u", SInstrGetBuu(pc[0]))

  // Local PC which we store back into `task` when returning
  SInstr *pc = task->pc;

  // Maximum number of operations a task can execute w/o yielding
  size_t execution_limit = 100;

  while (1) {
    switch (SInstrGetOP(pc[0])) {
    case S_OP_RETURN: {
      SDLogOpAB;
      ++pc;
      task->pc = (pc == task->end) ? task->start : pc;
      return STaskStatusEnd;
    }
    case S_OP_MOVE: {
      SDLogOpAB;
      ++pc; break;
    }
    case S_OP_LOADK: {
      SDLogOpABu;
      ++pc; break;
    }
    case S_OP_ADDI: {
      SDLogOpABC;
      ++pc; break;
    }
    case S_OP_SUBI: {
      SDLogOpABC;
      ++pc; break;
    }
    case S_OP_MULI: {
      SDLogOpABC;
      ++pc; break;
    }
    case S_OP_DIVI: {
      SDLogOpABC;
      ++pc; break;
    }
    case S_OP_NOT: {
      SDLogOpAB;
      ++pc; break;
    }
    case S_OP_EQ: {
      SDLogOpABC;
      ++pc; break;
    }
    case S_OP_LT: {
      SDLogOpABC;
      ++pc; break;
    }
    case S_OP_LE: {
      SDLogOpABC;
      ++pc; break;
    }
    case S_OP_JUMP: {
      SDLogOpBss;
      ++pc; break;
    }
    case S_OP_YIELD: {
      SDLogOpAB;
      task->pc = ++pc;
      return STaskStatusYield;
    }
    default:
      S_UNREACHABLE;
    }

    // Reached execution limit?
    if (execution_limit == 0) {
      printf("[vm] %p execution limit -- yielding\n", task);
      task->pc = ++pc;
      return STaskStatusYield;
    } else {
      --execution_limit;
    }
  }

  task->pc = ++pc;
  return STaskStatusError;

  // Dispatch first instruction
  goto *oct[SInstrGetOP(pc[0])];

on__RETURN:
  SDLogOpAB;
  ++pc;
  task->pc = (pc == task->end) ? task->start : pc;
  return STaskStatusEnd;

on__MOVE:
  SDLogOpAB;
  goto *oct[SInstrGetOP((++pc)[0])];

on__LOADK:
  SDLogOpABu;
  goto *oct[SInstrGetOP((++pc)[0])];

on__ADDI:
  SDLogOpABC;
  goto *oct[SInstrGetOP((++pc)[0])];

on__SUBI:
  SDLogOpABC;
  goto *oct[SInstrGetOP((++pc)[0])];

on__MULI:
  SDLogOpABC;
  goto *oct[SInstrGetOP((++pc)[0])];

on__DIVI:
  SDLogOpABC;
  goto *oct[SInstrGetOP((++pc)[0])];

on__NOT:
  SDLogOpAB;
  goto *oct[SInstrGetOP((++pc)[0])];

on__EQ:
  SDLogOpABC;
  goto *oct[SInstrGetOP((++pc)[0])];

on__LT:
  SDLogOpABC;
  goto *oct[SInstrGetOP((++pc)[0])];

on__LE:
  SDLogOpABC;
  goto *oct[SInstrGetOP((++pc)[0])];

on__JUMP:
  SDLogOpBss;
  goto *oct[SInstrGetOP((++pc)[0])];

on__YIELD:
  SDLogOpAB;
  task->pc = ++pc;
  return STaskStatusYield;
}