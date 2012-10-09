#ifndef S_TASK_H_
#define S_TASK_H_
#include <sol/common.h>
#include <sol/instr.h>

typedef enum {
  STaskStatusEnd = 0, // The task has ended normally
  STaskStatusError,   // The task was interrupted by a fault
  STaskStatusYield,   // The task has yielded
} STaskStatus;

// A task is an instance of a program and is executed by a scheduler
typedef struct STask {
  SInstr *start;      // Oldest available instruction
  SInstr *pc;         // Current instruction (Program Counter)
  SInstr *end;        // Last available instruction.
  struct STask* next; // Next task. Used by scheduler.
  // TODO: Registry
} STask;

inline static STask* STaskCreate(SInstr* instrv, size_t instrc) {
  STask* t = (STask*)malloc(sizeof(STask));
  t->start = instrv;
  t->pc = instrv;
  t->end = instrv+(instrc-1);
  t->next = 0;
  return t;
}

inline static void STaskDestroy(STask* t) {
  free((void*)t);
}

#endif // S_TASK_H_
