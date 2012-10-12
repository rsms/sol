// Scheduler -- maintains a run queue of tasks which are executed in order.
//
// When a task's execution is suspended with a "yield" status, the task is
// placed at the end of the queue so that it can run again once other queued
// tasks have had a chance to run. When a task is suspended with a "end" or
// "error" status, the task is removed from the scheduler (unscheduled).
//
// To run a task, schedule it by calling `SSchedTask` and then enter the
// scheduler's runloop by calling `SSchedRun`. `SSchedRun` will return when all
// queued tasks have been unscheduled.
//
#ifndef S_SCHED_H_
#define S_SCHED_H_
#include <sol/common.h>
#include <sol/runq.h>
#include <sol/vm.h>

// Task scheduler
typedef struct {
  SRunQ runq;
} SSched;

// Create a new scheduler
SSched* SSchedCreate();

// Destroy a scheduler. Effectively calls `STaskDestroy` on each task that is
// still in the run queue.
void SSchedDestroy(SSched* s);

// Schedule a task `t` by adding it to the end of the run queue of scheduler
// `s`. It's important not to schedule tasks that have already been scheduled.
// Doing so might very well cause a crash.
inline static void SSchedTask(SSched* s, STask* t) {
  assert(t->next == 0);
  SRunQPushTail(&s->runq, t);
}

// Run the scheduler. This function exits when the run queue is empty.
void SSchedRun(SVM* vm, SSched* s);

// Debugging: Dumps the state of scheduler `s` to stdout.
void SSchedDump(SSched* s);

#endif // S_SCHED_H_
