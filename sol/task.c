#include "task.h"
#include "log.h"

const STask STaskZombie = {0, 0, 0, 0};
const STask STaskRoot = {0, 0, 0, 0};

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

STask* STaskCreate(SFunc* func, STask* supert) {
  STask* t = (STask*)malloc(sizeof(STask));
  t->parent = (supert == 0) ? (STask*)&STaskRoot : supert;
  t->next = 0;
  t->ar = SARecCreate(func, 0);
  t->wp = 0;
  t->refc = 1;
  t->wtype = 0;
  return t;
}

void STaskDestroy(STask* t) {
  SLogD("STaskDestroy %p", t);
  free((void*)t);
}
