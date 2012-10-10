#include "task.h"

STask* STaskCreate(SInstr* instrv, size_t instrc) {
  STask* t = (STask*)malloc(sizeof(STask));
  t->start = instrv;
  t->pc = instrv;
  //t->end = instrv+(instrc-1);
  t->next = 0;
  return t;
}

void STaskDestroy(STask* t) {
  free((void*)t);
}
