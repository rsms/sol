// Scheduler -- maintains a queue of tasks which are executed in order of being
// queued. A task's code is executed by the Virtual Machine. When a task's
// execution is suspended with a "yield" status, the task is placed at the end
// of the queue so that it can run again once other queued tasks have had a
// chance to run. When a task is suspended with a "end" or "error" status, the
// task is removed from the scheduler (unscheduled).
//
// To run a task, schedule it by calling `SSchedEnqueue` and then enter the
// scheduler's runloop by calling `SSchedRunLoop`. `SSchedRunLoop` will return
// when all queued tasks have been unscheduled.
//
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

SSched* SSchedCreate();
void SSchedDestroy(SSched* s);

// Add to tail (end of queue)
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

// Remove head (first in queue)
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
