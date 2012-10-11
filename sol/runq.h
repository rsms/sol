// Run queue -- maintains a queue of tasks which are usually executed by a
// Scheduler in order from head to tail.
#ifndef S_RUNQ_H_
#define S_RUNQ_H_
#include <sol/common.h>
#include <sol/task.h>

typedef struct {
  STask* head;  // Highest priority task
  STask* tail;  // Lowest priority task
  size_t count; // Number of queued tasks
} SRunQ;

// Constant initializer
#define S_RUNQ_INIT (SRunQ){0, 0, 0}

// Initialize a run queue
inline static void SRunQInit(SRunQ* q) {
  q->head = 0;
  q->tail = 0;
  q->count = 0;
}

// Add to tail (end of queue)
inline static void SRunQPushTail(SRunQ* q, STask* t) {
  // 1. []       --> [A -> x]
  // 2. [A -> x] --> [A -> B -> x]
  //    ...
  STask* tail = q->tail;
  if (tail == 0) {
    q->head = t;
  } else {
    tail->next = t;
  }
  q->tail = t;
  ++q->count;
}

// Remove head (first in queue)
inline static STask* SRunQPopHead(SRunQ* q) {
  // 1. [A -> x]      --> []
  // 2. [A -> B -> x] --> [B -> x]
  //    ...
  STask* t = q->head;
  if (t == q->tail) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
  } else {
    q->head = t->next;
    --q->count;
  }
  return t;
}

// Dequeue each task
#define SRunQPopEachHead(q, t) \
  for (; ((t) = (q)->head) != 0; (q)->head = (t)->next)

// Move head to tail
inline static void SRunQPopHeadPushTail(SRunQ* q) {
  // 1. [A -> x]      --> [A -> x] (noop)
  // 2. [A -> B -> x] --> [B -> A -> x]
  //    ...
  STask* t = q->head;
  if (q->tail != t) {
    q->tail->next = t;
    q->head = t->next;
    t->next = 0;
    q->tail = t;
  }
}

#endif // S_RUNQ_H_
