#include "sched.h"
#include "instr.h"

#if S_DEBUG
void SSchedDump(SSched* s) {
  printf("[sched %p] run queue:", s);
  STask* t = s->runq.head;
  size_t count = 0;
  while (t != 0) {
    printf(
      "%s[task %p]%s",
      (count++ % 4 == 0) ? "\n  " : "",
      t,
      (t->next) ? " -> " : ""
    );
    t = t->next;
  }
  printf("\n");
}
#else
#define SSchedDump(x) ((void)0)
#endif

SSched* SSchedCreate() {
  SSched* s = (SSched*)malloc(sizeof(SSched));
  s->runq = S_RUNQ_INIT;
  return s;
}

void SSchedDestroy(SSched* s) {
  STask* t;
  SRunQPopEachHead(&s->runq, t) {
    STaskDestroy(t);
  }
  free((void*)s);
}

// For the sake of readability and structure, the `SSchedExec` function is
// defined in a separate file
#include "sched_exec.h"

void SSchedRun(SSched* s) {
  STask* t;
  SRunQ* runq = &s->runq;

  SSchedDump(s);

  while ((t = runq->head) != 0) {
    //printf("[sched] resuming <STask %p>\n", t);
    int ts = SSchedExec(t);

    switch (ts) {
      case STaskStatusError:
      // task ended from an error. Remove from scheduler.
      printf("Program error\n");
      SRunQPopHead(runq);
      STaskDestroy(t);
      break;

    case STaskStatusEnd:
      // task ended. Remove from scheduler.
      SRunQPopHead(runq);
      STaskDestroy(t);
      break;

    case STaskStatusYield:
      SRunQPopHeadPushTail(runq);
      break;

    default:
      S_UNREACHABLE;
    }
  }

}
