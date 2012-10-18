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

typedef uint32_t STaskID;
#define STaskIDRoot 0
#define STaskIDMax UINT32_MAX

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
  STaskWaitMsg,         // Waiting for a message to arrive in its inbox
};

// Inter-task message
typedef struct SMsg {
  struct SMsg* parent; // List link
  STaskID      sender; // tid of the task that sent this message
  SValue       value;  // Value of the message
} SMsg;

typedef struct S_PACKED STask {
  STaskID       tid;    // ID of this task. Must be first member
  STaskID       ptid;   // ID of the task that spawned this task

  struct STask* next;   // Next task (used by scheduler queues)
  struct STask* prev;   // Previous task (used by scheduler queues)
  
  SARec*        ar;     // Current (top-of stack) AR, singly-linked LIFO list
  
  void*         wp;     // Something the task is waiting for
  STaskWait     wtype;  // Type of thing the task is waiting for
} STask;

STask* STaskCreate(SFunc* func, STaskID ptid);
void STaskDestroy(STask* t);

#endif // S_TASK_H_
