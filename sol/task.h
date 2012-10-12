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

typedef enum {
  STaskStatusError = 0, // The task was interrupted by a fault
  STaskStatusEnd,       // The task ended (and has been reset)
  STaskStatusYield,     // The task yielded
  STaskStatusWait,      // The task yielded b/c it's waiting for I/O or timer
} STaskStatus;

typedef struct STask {
  SInstr* start;      // Oldest available instruction
  SInstr* pc;         // Current instruction (Program Counter)
  SValue* constants;  // Constants accessed by instructions
  struct STask* next; // Next task. Used by scheduler.
  SValue registry[10];
} STask;

STask* STaskCreate(SInstr* instrv, SValue* constants);
void STaskDestroy(STask* t);

#endif // S_TASK_H_
