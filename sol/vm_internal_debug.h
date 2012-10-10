// Note: This file should only be included by vm.c

// Instruction debug logging macros
//
// void SVMDLogI(const char* format, ...)
// -> "[vm] %taskid %absicount " format ... "\n"
// 
//
#if S_VM_DEBUG_LOG
  #define SVMDLog(fmt, ...) printf("[vm] %p " fmt "\n", task, ##__VA_ARGS__)
  S_UNUSED static const char const* _debug_op_names[] = {
    #define OP_TABLE(name, _) #name,
    S_INSTR_DEFINE(OP_TABLE)
    #undef  OP_TABLE
  };
#else  // S_VM_DEBUG_LOG
  #define SVMDLog(...) ((void)0)
#endif // S_VM_DEBUG_LOG

#define SVMDLogI(fmt, ...) SVMDLog("%-10zu " fmt, \
  ((size_t)((pc) - (task->start))), ##__VA_ARGS__)

#define SVMDLogOp(fmt, ...) \
  SVMDLogI("[%02u] %-6s " fmt, \
    (uint8_t)SInstrGetOP(*pc), _debug_op_names[SInstrGetOP(*pc)], \
    ##__VA_ARGS__ \
  )

#define SVMDLogOpA SVMDLogOp(    " A:     %3u", (uint8_t)SInstrGetA(*pc))
#define SVMDLogOpAB() SVMDLogOp( " A,B:   %3u, %3u", \
  (uint8_t)SInstrGetA(*pc), (uint16_t)SInstrGetB(*pc))
#define SVMDLogOpABC() SVMDLogOp(" A,B,C: %3u, %3u, %3u", \
  (uint8_t)SInstrGetA(*pc), (uint16_t)SInstrGetB(*pc), \
  (uint16_t)SInstrGetC(*pc))
#define SVMDLogOpABs() SVMDLogOp(" A,Bs:  %3u, %6d", \
  (uint8_t)SInstrGetA(*pc), SInstrGetBs(*pc))
#define SVMDLogOpABu() SVMDLogOp(" A,Bu:  %3u, %6u", \
  (uint8_t)SInstrGetA(*pc), SInstrGetBu(*pc))
#define SVMDLogOpBss() SVMDLogOp(" Bss:   %8d", SInstrGetBss(*pc))
#define SVMDLogOpBuu() SVMDLogOp(" Buu:   %8u", SInstrGetBuu(*pc))
