#include "task.h"

SARec* SARecCreate(SFunc* func, SARec* parent) {
  SARec* ar = (SARec*)malloc(sizeof(SARec));
  ar->func = func;
  
  // We have to set PC to instrv MINUS ONE since the VM executive sees PC as
  // the instruction that WAS executed and thus the first thing that happens
  // when a function runs is PC++, putting us at the first instruction.
  ar->pc = func->instructions -1;

  ar->parent = parent;
  return ar;
}

void SARecDestroy(SARec* ar) {
  free((void*)ar);
}

STask* STaskCreate(SFunc* func) {
  STask* t = (STask*)malloc(sizeof(STask));
  t->next = 0;
  t->ar = SARecCreate(func, 0);
  return t;
}

void STaskDestroy(STask* t) {
  free((void*)t);
}
