#include "sched.h"
#include "vm.h"

void SSchedDump(SSched* s) {
  printf("SSched %p\n  head: ", s);
  STask* t = s->qhead;
  while (t != 0) {
    printf("<STask %p> -> ", t);
    t = t->next;
  }
  printf("nil.\n  tail: ");
  if (s->qtail != 0) {
    printf("<STask %p>\n", s->qtail);
  } else {
    printf("nil\n");
  }
}

SSched* SSchedCreate() {
  SSched* s = (SSched*)malloc(sizeof(SSched));
  s->qhead = 0;
  s->qtail = 0;
  s->count = 0;
  return s;
}

void SSchedDestroy(SSched* s) {
  STask* t;
  while ((t = s->qhead) != 0) {
    s->qhead = t->next;
    STaskDestroy(t);
  }
  free((void*)s);
}

void SSchedRunLoop(SSched* s) {
  STask* t;
  SSchedDump(s);

  while ((t = s->qhead) != 0) {
    //printf("[sched] resuming <STask %p>\n", t);
    int ts = SVMExec(t);
    switch (ts) {
      case STaskStatusError:
      // task ended from an error. Remove from scheduler.
      printf("Program error\n");
      SSchedDequeue(s);
      STaskDestroy(t);
      break;

    case STaskStatusEnd:
      // task ended. Remove from scheduler.
      SSchedDequeue(s);
      STaskDestroy(t);
      break;

    case STaskStatusYield:
      SSchedRequeue(s);
      break;

    default:
      S_UNREACHABLE;
    }
  }

}
