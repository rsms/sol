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
#include <sol/task.h>
#include <sol/vm.h>

// Task scheduler
typedef struct {
  STask* rhead;   // Run queue queue head
  STask* rtail;   // Run queue queue tail
  STask* whead;   // Waiting queue head
  STask* wtail;   // Waiting queue tail
  void*  events_;
} SSched;

// Create a new scheduler
SSched* SSchedCreate();

// Destroy a scheduler. Effectively calls `STaskDestroy` on each task that is
// still in the run queue.
void SSchedDestroy(SSched* s);

// Schedule a task `t` by adding it to the end of the run queue of scheduler
// `s`. It's important not to schedule tasks that have already been scheduled.
void SSchedTask(SSched* s, STask* t);

// Run the scheduler. This function exits when the run queue is empty.
void SSchedRun(SVM* vm, SSched* s);

#endif // S_SCHED_H_
