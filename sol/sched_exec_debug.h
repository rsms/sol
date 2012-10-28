// Note: This file should only be included by vm.c

// Instruction debug logging macros
//
// void SVMDLogI(const char* format, ...)
// -> "[vm] %taskid %absicount " format ... "\n"
// 
//
#if S_VM_DEBUG_LOG
  #define SVMDLog(fmt, ...) SLog("[vm] %-14p %-14p " fmt, \
    task, task->ar->func, ##__VA_ARGS__)

  S_UNUSED static const char const* _debug_op_names[] = {
    #define OP_TABLE(name, _) #name,
    S_INSTR_DEFINE(OP_TABLE)
    #undef  OP_TABLE
  };

  #define SVMDLogRRVal(reg, index) do { \
    char __bufv[32]; \
    SLogD("[vm] R(%u) = %s", (index), SValueRepr(__bufv, 32, &(reg)[(index)]));\
  } while (0)

  #define SVMDLogRVal(r) SVMDLogRRVal(registry, r)

  #define SVMDLogKVal(k) do { \
    char __bufv[32]; \
    SLogD("[vm] K(%u) = %s", (k), SValueRepr(__bufv, 32, &constants[(k)])); \
  } while (0)

  #define SVMDLogRKVal(rk) do { \
    if (rk < 255) { \
      SVMDLogRVal(rk); \
    } else {\
      SVMDLogKVal(rk - 255); \
    } \
  } while (0)

  #define SVMDLogInstrRVal(regname, i) do { \
    uint16_t r = SInstrGet##regname((i)); \
    SVMDLogRVal(r); \
  } while (0)

  #define SVMDLogInstrKVal(regname, i) do { \
    uint16_t k = SInstrGet##regname((i)); \
    SVMDLogKVal(k); \
  } while (0)

  #define SVMDLogInstrRKVal(regname, i) do { \
    uint16_t rk = SInstrGet##regname((i)); \
    SVMDLogRKVal(rk); \
  } while (0)

// SLogD("R(%u) = %s", SInstrGetA(*pc), SValueRepr(__buf_##__LINE__, 32, &K_B(*pc)));

#else  // S_VM_DEBUG_LOG
  #define SVMDLog(...) ((void)0)
  #define SVMDLogRRVal(reg, index) ((void)0)
  #define SVMDLogRVal(r) ((void)0)
  #define SVMDLogKVal(k) ((void)0)
  #define SVMDLogInstrRVal(regname, i) ((void)0)
  #define SVMDLogInstrKVal(regname, i) ((void)0)
  #define SVMDLogInstrRKVal(regname, i) ((void)0)
#endif // S_VM_DEBUG_LOG

#define SVMDLogI(fmt, ...) SVMDLog("%-10zu " fmt, \
  ((size_t)((pc) - (ar->func->instructions))), ##__VA_ARGS__)

#define SVMDLogOp(fmt, ...) \
  SVMDLogI("%-6s " fmt, _debug_op_names[SInstrGetOP(*pc)], ##__VA_ARGS__)

#define SVMDLogOpA SVMDLogOp(    " A:   %3u", (uint8_t)SInstrGetA(*pc))
#define SVMDLogOpAB() SVMDLogOp( " AB:  %3u, %3u", \
  (uint8_t)SInstrGetA(*pc), (uint16_t)SInstrGetB(*pc))
#define SVMDLogOpABC() SVMDLogOp(" ABC: %3u, %3u, %3u", \
  (uint8_t)SInstrGetA(*pc), (uint16_t)SInstrGetB(*pc), \
  (uint16_t)SInstrGetC(*pc))
#define SVMDLogOpABs() SVMDLogOp(" ABs: %3u, %6d", \
  (uint8_t)SInstrGetA(*pc), SInstrGetBs(*pc))
#define SVMDLogOpABu() SVMDLogOp(" ABu: %3u, %6u", \
  (uint8_t)SInstrGetA(*pc), SInstrGetBu(*pc))
#define SVMDLogOpBss() SVMDLogOp(" Bss: %8d", SInstrGetBss(*pc))
#define SVMDLogOpBuu() SVMDLogOp(" Buu: %8u", SInstrGetBuu(*pc))
