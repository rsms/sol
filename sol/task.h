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
  STaskStatusError = 0, // The task was interrupted by a fault
  STaskStatusEnd,       // The task ended (and has been reset)
  STaskStatusYield,     // The task yielded
  STaskStatusWait,      // The task yielded, waiting for I/O
  STaskStatusTimer,     // The task yielded, waiting for a timer
} STaskStatus;

typedef struct STask {
  SFunc*  func;       // Main function
  SInstr* pc;         // Current instruction (Program Counter)
  struct STask* next; // Next task. Used by scheduler.
  SValue registry[10];
} STask;

STask* STaskCreate(SFunc* func);
void STaskDestroy(STask* t);

#endif // S_TASK_H_
