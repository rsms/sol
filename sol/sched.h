#ifndef S_SCHED_H_
#define S_SCHED_H_
#include <sol/common.h>
#include <sol/task.h>

// Task scheduler
typedef struct {
  STask* qhead; // Highest priority task
  STask* qtail; // Lowest priority task
  size_t count; // Number of queued tasks
} SSched;

inline static SSched* SSchedCreate() {
  SSched* s = (SSched*)malloc(sizeof(SSched));
  s->qhead = 0;
  s->qtail = 0;
  s->count = 0;
  return s;
}

inline static void SSchedDestroy(SSched* s) {
  STask* t;
  while ((t = s->qhead) != 0) {
    s->qhead = t->next;
    STaskDestroy(t);
  }
  free((void*)s);
}

inline static void SSchedEnqueue(SSched* s, STask* t) {
  // 1. []       --> [A -> x]
  // 2. [A -> x] --> [A -> B -> x]
  //    ...
  STask* tail = s->qtail;
  if (tail == 0) {
    s->qhead = t;
  } else {
    tail->next = t;
  }
  s->qtail = t;
  ++s->count;
}

inline static STask* SSchedDequeue(SSched* s) {
  // 1. [A -> x]      --> []
  // 2. [A -> B -> x] --> [B -> x]
  //    ...
  STask* t = s->qhead;
  if (t == s->qtail) {
    s->qhead = 0;
    s->qtail = 0;
    s->count = 0;
  } else {
    s->qhead = t->next;
    --s->count;
  }
  return t;
}

// Move head to tail
inline static void SSchedRequeue(SSched* s) {
  // 1. [A -> x]      --> [A -> x] (noop)
  // 2. [A -> B -> x] --> [B -> A -> x]
  //    ...
  STask* t = s->qhead;
  if (s->qtail != t) {
    s->qtail->next = t;
    s->qhead = t->next;
    t->next = 0;
    s->qtail = t;
  }
}

void SSchedRunLoop(SSched* s);
void SSchedDump(SSched* s);

#endif // S_SCHED_H_
