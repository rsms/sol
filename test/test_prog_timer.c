// Tests programming with timers.
// Covers YIELD(1), DBGCB, event sequence, and run- & wait queue integrity.
#include "test.h"
#include <sol/vm.h>
#include <sol/sched.h>

STask* task1; // Schedules a timer, waits for it and is finally resumed 
STask* task2; // Scheduled after task1 to inspect the VM state
int sequence = 0; // For verifying call sequence

void task1_on_resumed(SVM* vm, SSched* s, STask* t, SInstr* pc) {
  // This is called as task1 is resumed after the timer triggered
  STrace();
  assert(t == task1);
  assert(sequence++ == 2);

  // AR's PC should be 1 less than the PC when this callback function is called.
  assert(t->ar->pc+1 == pc);

  // AR's PC should be at the first YIELD instruction
  assert(SInstrGetOP(*t->ar->pc) == S_OP_YIELD);
  assert(SInstrGetA(*t->ar->pc) == 1); // wait for timer

  // Verify that task1 is the only task in the run queue (task2 has already
  // ended)
  assert(s->rhead == task1);
  assert(s->rtail == task1);
  assert(task1->next == 0);

  // Verify that the wait queue is empty
  assert(s->whead == 0);
  assert(s->wtail == 0);

  assert(sequence++ == 3);
}

void task2_on_task1_suspended(SVM* vm, SSched* s, STask* t, SInstr* pc) {
  // This is called in task2 just after task1 is suspended, waiting for a timer
  STrace();
  assert(t == task2);
  assert(sequence++ == 0);

  // Verify that task2 is the only task in the run queue
  assert(s->rhead == task2);
  assert(s->rtail == task2);
  assert(task2->next == 0);

  // Verify that task1 is the only task in the wait queue
  assert(s->whead == task1);
  assert(s->wtail == task1);
  assert(task1->next == 0);

  // Verify that task1 is waiting for a timer
  assert(task1->wtype == STaskWaitTimer);
  assert(task1->wp != 0);

  assert(sequence++ == 1);
}

void test_timer(SVM* vm) {
  // Covered: YIELD(1)
  SValue constants1[] = {
    SValueNumber(5),
    SValueOpaque(&task1_on_resumed),
  };
  SInstr instructions1[] = {
    SInstr_YIELD(1, S_INSTR_RK_k+0, 0),
    SInstr_DBGCB(0, 1, 0), // ccall K(B)(vm, s, t, pc)
    SInstr_RETURN(0, 0),
  };
  SFunc* func1 = SFuncCreate(constants1, instructions1);
  task1 = STaskCreate(func1, 0, 0);

  // A task that is scheduled just after task1, inspecting VM state
  SValue constants2[] = {
    SValueOpaque(&task2_on_task1_suspended),
  };
  SInstr instructions2[] = {
    SInstr_DBGCB(0, 0, 0), // ccall K(B)(vm, s, t, pc)
    SInstr_RETURN(0, 0),
  };
  SFunc* func2 = SFuncCreate(constants2, instructions2);
  task2 = STaskCreate(func2, 0, 0);

  // Make a scheduler and add the tasks
  SSched* sched = SSchedCreate();
  SSchedTask(sched, task1);
  SSchedTask(sched, task2);

  // Verify run queue is: -> task1 <-> task2 <-
  assert(sched->rhead == task1);
  assert(task1->next == task2);
  assert(task1->prev == 0);
  assert(sched->rtail == task2);
  assert(task2->prev == task1);
  assert(task2->next == 0);

  // Verify that wait queue is empty
  assert(sched->whead == 0);
  assert(sched->wtail == 0);

  // Run
  SSchedRun(vm, sched);

  assert(sequence == 4);

  SSchedDestroy(sched);
  STaskRelease(task1);
  STaskRelease(task2);
  SFuncDestroy(func1);
  SFuncDestroy(func2);
}

int main(int argc, const char** argv) {
  SVM vm = SVM_INIT;

  test_timer(&vm);

  return 0;
}
