#ifndef S_FUNC_T_
#define S_FUNC_T_

#include <sol/value.h>
#include <sol/instr.h>

typedef struct {
  SValue* constants;
  SInstr* instructions;
} SFunc;

SFunc* SFuncCreate(SValue* constants, SInstr* instructions);
void SFuncDestroy(SFunc* f);

#endif // S_FUNC_T_