// Task -- represents a program that can be run. Tasks are supposed to be very
// cheap so to allow creation of large amounts of tasks. A task can be seen as a
// "thread" or "coroutine". A task has program code and a program counter (PC).
// The PC is the "cursor" to the instruction-to-be executed next time the task
// is scheduled by a scheduler and executed by the virtual machine. The virtual
// machine will advance the task's PC as it executes a task's instructions.
//
#ifndef S_TASK_H_
#define S_TASK_H_
#include <sol/common.h>
#include <sol/instr.h>
#include <sol/value.h>
#include <sol/func.h>

typedef enum {
  STaskStatusYield = 0, // The task yielded (is rescheduled)
  STaskStatusError,     // The task ended from a fault
  STaskStatusEnd,       // The task ended normally
  STaskStatusSuspend,   // The task is suspended (e.g. waiting for I/O or timer)
} STaskStatus;

typedef uint8_t STaskWait;
enum {
  STaskWaitTimer = 0,   // Waiting for a timer
};

// An activation record (or call-stack frame)
typedef struct SARec {
  SFunc*        func;         // Function
  SInstr*       pc;           // PC
  SValue        registry[10]; // Registry
  struct SARec* parent;       // Parent AR
} SARec;

SARec* SARecCreate(SFunc* func, SARec* parent);
void SARecDestroy(SARec* ar);

typedef struct S_PACKED STask {
  struct STask* parent; // Task that spawned this task
  struct STask* next;   // Next task (used by scheduler queues)
  struct STask* prev;   // Previous task (used by scheduler queues)
  SARec*        ar;     // Current (top-of stack) AR, singly-linked LIFO list
  void*         wp;     // Something the task is waiting for
  uint32_t      refc;   // Number of live subtasks of this task
  STaskWait     wtype;  // Type of thing the task is waiting for
} STask;

// Special constant value that represent the "task is zombie" marker. When a
// task dies, it's `next` member is set to this constant.
const STask STaskZombie;

// Special constant value that represent the root task. Tasks spawned by the
// system will have this as its parent.
const STask STaskRoot;

STask* STaskCreate(SFunc* func, STask* supert);
void STaskDestroy(STask* t);

// Increment and decrement reference count
static inline void S_ALWAYS_INLINE STaskRetain(STask* t) {
  ++t->refc;
}
static inline bool S_ALWAYS_INLINE STaskRelease(STask* t) {
  if (--t->refc == 0) {
    STaskDestroy(t);
    return true;
  } else {
    return false;
  }
}

#endif // S_TASK_H_
