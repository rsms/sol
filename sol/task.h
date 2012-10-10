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

typedef enum {
  STaskStatusEnd = 0, // The task ended (and has been reset)
  STaskStatusError,   // The task was interrupted by a fault
  STaskStatusYield,   // The task yielded
} STaskStatus;

typedef struct STask {
  SInstr *start;      // Oldest available instruction
  SInstr *pc;         // Current instruction (Program Counter)
  struct STask* next; // Next task. Used by scheduler.
  // TODO: Registry
} STask;

STask* STaskCreate(SInstr* instrv, size_t instrc);
void STaskDestroy(STask* t);

#endif // S_TASK_H_
