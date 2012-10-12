#include "task.h"

STask* STaskCreate(SInstr* instrv, SValue* constants) {
  STask* t = (STask*)malloc(sizeof(STask));
  t->start = instrv;
  t->pc = instrv;
  t->constants = constants;
  t->next = 0;
  return t;
}

void STaskDestroy(STask* t) {
  free((void*)t);
}
