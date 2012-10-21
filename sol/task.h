// Task -- represents a program that can be run. Tasks are supposed to be very
// cheap so to allow creation of large amounts of tasks. A task can be seen as a
// "thread" or "coroutine". A task has program code and a program counter (PC).
// The PC is the "cursor" to the instruction-to-be executed next time the task
// is scheduled by a scheduler and executed by the virtual machine. The virtual
// machine will advance the task's PC as it executes a task's instructions.
#ifndef S_TASK_H_
#define S_TASK_H_
#include <sol/common.h>
#include <sol/arec.h>
#include <sol/msg.h>

// Status of a task, returned by SSchedExec after executing a task
typedef enum {
  STaskStatusYield = 0, // The task yielded (is rescheduled)
  STaskStatusError,     // The task ended from a fault
  STaskStatusEnd,       // The task ended normally
  STaskStatusSuspend,   // The task is suspended (e.g. waiting for I/O or timer)
} STaskStatus;

// Type of thing a task is waiting for (value of a task's `wtype` member)
typedef uint8_t STaskWait;
enum {
  STaskWaitTimer = 0,   // Waiting for a timer
  STaskWaitMsg,         // Waiting for a message to arrive to its inbox
};

// Various flags set for a task
typedef uint32_t STaskFlag;
enum {
  // Normally when a subtask exits abnormally, the exit propagates to the
  // supertask, causing the supertask to exit with the same status. If the
  // STaskFlagTrapExit flag is set however, the supertask will instead receive
  // a message "subtask exited abnormally".
  STaskFlagTrapExit = 1,
};

typedef struct S_PACKED STask {
  struct STask* volatile next; // Next task (used by scheduler queues)
  struct STask*     prev;   // Previous task (used by scheduler queues)

  SARec*            ar;     // Call stack top. Singly-linked LIFO list

  struct STask*     supt;   // Our supertask -- task that spawned us
  volatile uint32_t refc;   // Number of live tasks that reference this task
  STaskFlag         flags;  // Flags

  void*             wp;     // Something the task is waiting for
  STaskWait         wtype;  // Type of thing the task is waiting for

  SMsgQ             inbox;  // Message inbox
} STask; // 113

STask* STaskCreate(SFunc* func, STask* supt, STaskFlag flags);
void STaskDestroy(STask* t);

// Special constant STask that gets assigned to the `next` member of tasks that
// have live subtasks. The subtasks are considered "zombies" in this case.
const STask STaskDead;

// Increment reference count
inline static void S_ALWAYS_INLINE STaskRetain(STask* t) {
  SAtomicAdd32((int32_t*)&t->refc, 1);
}

// Decrement reference count. Returns true if `t` was free'd
inline static bool S_ALWAYS_INLINE STaskRelease(STask* t) {
  if (SAtomicSubAndFetch(&t->refc, 1) == 0) {
    STaskDestroy(t);
    return true;
  } else {
    return false;
  }
}

#endif // S_TASK_H_
