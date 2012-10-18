#include "task.h"
#include "log.h"

// `gNextTaskID` -- Monotonic counter that "returns" the next task id.
//
// TODO: This needs to be something fancier. Imagine a scenario where we spawn
// task A(tid=1), then A spawns new tasks (that execute and die) on a regular
// schedule. Eventually this counter will wrap around (after STaskIDMax-1 tasks)
// causing the next task to be assigned 0 (the special "root tid") and possibly
// even worse, the following task is assigned tid 1, now we have two tasks
// running with the same tid, which breaks our promise of tids being unique.
//
// Either we want a "free list" approach where when a task is destroyed, it's
// tid is placed in the "free" list of tids that can later be assigned to new
// tasks. However this approach is dangerous since imagine task A spawning task
// B where task B starts waiting for a message from A. Since there's no notion
// of "wait for a message from task x", B will wait for any message and then
// compare the message's sender tid. Now, say task A dies, and new task C starts
// which gets assigned the tid that A previously had. C knows about B and sends
// B a message. B will compare the tid and wrongfully think that C is actually
// A. This can lead to subtle bugs. C is able to message B since tids are
// sequential.
//
// Another approach is to generate "secure" tids, much like Erlang does. In that
// case what we described earlier wouldn't matter, since C would not be able to
// guess the tid of B, so be would--with very high likeliness--never receive a
// message from that tid again. There are two major hurdles with secure tid
// generation:
//  1. For them to be truly secure, a source of random data is needed which is
//     usually a costly operation to query.
//  2. Collision detection needs to happen, for the unlikely scenario that a tid
//     is generated that is identical to a tid of a live task.
// For 1. we could use a pseudo-random number generator instead of using a
// source of true random data. This could be efficiently implemented where the
// one requirement is that the "seed" of the generation can not be manipulated
// to a certain state by any one or set of tasks (if it was, a task would be
// able to predict the next tid and the system would be insecure). For 2, well,
// we need to figure that one out. Or we could simpy base the tid on the task's
// memory address, but that would make the tid type unnecessarily large.
// 
volatile STaskID gNextTaskID = 0;

STask* STaskCreate(SFunc* func, STaskID ptid) {
  STask* t = (STask*)malloc(sizeof(STask)); // TODO: malloc

  // Scheduler doubly-linked list links
  t->next = 0;
  t->prev = 0;

  // Entry activation record
  t->ar = SARecCreate(func, 0);

  // Assign a new task ID and store parent's task ID
  t->tid = ++gNextTaskID; // TODO: CAS
  t->ptid = ptid;

  // waiting for nothing
  t->wp = 0;
  t->wtype = 0;

  return t;
}

void STaskDestroy(STask* t) {
  SLogD("STaskDestroy %p", t);
  free((void*)t); // TODO
}
