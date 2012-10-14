#include "func.h"

SFunc* SFuncCreate(SValue* constants, SInstr* instructions) {
  SFunc* f = (SFunc*)malloc(sizeof(SFunc));
  f->constants = constants;
  f->instructions = instructions;
  return f;
}

void SFuncDestroy(SFunc* f) {
  free((void*)f);
}


