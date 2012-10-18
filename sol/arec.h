// An activation record (or call-stack frame) is created for each instance of a
// function call. It contains the function prototype that is is executing, a PC
// pointing to the current instruction in the function's code that is executing,
// a reference to the parent AR (link in a linked list forming the call stack),
// and finally a register that is used to store values during execution.
#ifndef S_AREC_H_
#define S_AREC_H_
#include <sol/common.h>
#include <sol/func.h>
#include <sol/instr.h>
#include <sol/value.h>

typedef struct SARec {
  SFunc*        func;         // Function
  SInstr*       pc;           // PC
  struct SARec* parent;       // Parent AR
  SValue        registry[10]; // Registry
} SARec;

// Create a new activation record. We inline this since it's only used in two
// places: Creation of a new task and when  calling a function. In the latter
// case we want to avoid a function call (thus this is inlined.)
inline static SARec* S_ALWAYS_INLINE
SARecCreate(SFunc* func, SARec* parent) {
  SARec* ar = (SARec*)malloc(sizeof(SARec));
  ar->func = func;
  
  // We have to set PC to instrv MINUS ONE since the VM executive sees PC as
  // the instruction that WAS executed and thus the first thing that happens
  // when a function runs is PC++, putting us at the first instruction.
  ar->pc = func->instructions -1;

  ar->parent = parent;
  return ar;
}

inline static void S_ALWAYS_INLINE
SARecDestroy(SARec* ar) {
  free((void*)ar);
}

#endif  // S_AREC_H_
