#include "task.h"
#include "msg.h"
#include "log.h"

const STask STaskDead = {0};

STask* STaskCreate(SFunc* func, STask* supt, STaskFlag flags) {
  STask* t = (STask*)malloc(sizeof(STask)); // TODO: malloc

  // Scheduler doubly-linked list links
  t->next = 0;
  t->prev = 0;

  // Entry activation record
  t->ar = SARecCreate(func, 0);

  // Set parent task, initialize refcount and STORE flags
  t->supt = supt;
  t->refc = 1; // 1 is our "live" refcount which is decremented when we end
  t->flags = flags;

  // waiting for nothing
  t->wp = 0;
  t->wtype = 0;

  // Initialize inbox
  t->inbox = S_MSGQ_INIT(t->inbox);

  return t;
}

void STaskDestroy(STask* t) {
  SLogD("STaskDestroy %p", t);
  if (t->ar) {
    SARecDestroy(t->ar);
  }
  free((void*)t); // TODO
}
