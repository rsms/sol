#include "task.h"

STask* STaskCreate(SFunc* func) {
  STask* t = (STask*)malloc(sizeof(STask));
  t->func = func;
  t->pc = func->instructions;
  t->next = 0;
  return t;
}

void STaskDestroy(STask* t) {
  free((void*)t);
}
