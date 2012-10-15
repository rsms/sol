#include "sched.h"
#include "instr.h"
#include "log.h"
#include "../deps/libev/ev.h"

typedef struct ev_loop EVLoop;

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
  s->events_ = (void*)ev_loop_new(EVFLAG_AUTO | EVFLAG_NOENV);

  // Set the scheduler as "userdata" of the evloop so that ev callbacks can
  // access the owning scheduler.
  ev_set_userdata((EVLoop*)s->events_, (void*)s);

  return s;
}

void SSchedDestroy(SSched* s) {
  STask* t;
  SRunQPopEachHead(&s->runq, t) {
    STaskDestroy(t);
  }
  ev_loop_destroy((EVLoop*)s->events_);
  free((void*)s);
}

typedef struct {
  ev_timer evtimer; // must be head
  STask* task;
} STimer;

static void TimerCallback(EVLoop *evloop, ev_timer *w, int revents) {
  SSched* s = (SSched*)ev_userdata(evloop);
  STimer* timer = (STimer*)w;
  SLogD("Timer triggered -- scheduling task");
  // Here, we could have some logic to schedule the task according to some
  // priority level. Right now, we are always scheduling the task at the end of
  // the runqueue. This means that events are processed in the order they arrive
  // in.
  SSchedTask(s, timer->task);

  free(w); // FIXME
}

// Start a timer
static inline STimer*
SSchedTimerStart(SSched* s, STask* task, SNumber after_ms, SNumber repeat_ms) {
  STimer* timer = (STimer*)malloc(sizeof(STimer)); // FIXME: malloc
  timer->task = task;

  ev_tstamp after_sec =
    (ev_tstamp)(after_ms == (SNumber)0) ? 0 :(after_ms / (SNumber)1000.0);
  
  ev_tstamp repeat_sec =
    (ev_tstamp)(repeat_ms == (SNumber)0) ? 0 :(repeat_ms / (SNumber)1000.0);

  ev_timer_init((ev_timer*)timer, TimerCallback, after_sec, repeat_sec);
  ev_timer_start((EVLoop*)s->events_, (ev_timer*)timer);
  SLogD("Timer scheduled to trigger after " SNumberFormat " ms", after_ms);

  return timer;
}

// For the sake of readability and structure, the `SSchedExec` function is
// defined in a separate file
#include "sched_exec.h"

void SSchedRun(SVM* vm, SSched* s) {
  STask* t;
  SRunQ* runq = &s->runq;
  EVLoop* evloop = (EVLoop*)s->events_;
  int* evrefs = ev_refcount(evloop);

  SSchedDump(s);

run_tasks:
  while ((t = runq->head) != 0) {
    //printf("[sched] resuming <STask %p>\n", t);
    #if S_VM_DEBUG_LOG
    printf(
        "[vm] ______________ ______________ __________ _______ ____ ______________\n"
        "[vm] Task           Function       PC         Op      Values\n"
    );// [vm] 0x7fd431c03930 1          [10] LT      ABC:   0, 255,   0
    #endif

    int ts = SSchedExec(vm, s, t);

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

    case STaskStatusWait:
      SRunQPopHead(runq);
      // But don't destroy as the task will be scheduled back in when whatever
      // it's waiting for has arrived
      break;

    default:
      S_UNREACHABLE;
    }

    // Handle any immediate events that triggered event watchers.
    // Conditional: Only if there are evrefs
    // Conditional: Only if there are queued tasks (otherwise we fall through
    // into `ev_run(evloop, 0)` below which would otherwise cause two calls).
    if (*evrefs > 0 && runq->head != 0) {
      SLogD("ev_run NOWAIT (%d refs)", *evrefs);
      ev_run(evloop, EVRUN_NOWAIT);
    }
  } // while there are queued tasks

  // While there are active event watchers
  if (*evrefs > 0) {
    ev_run(evloop, 0);
    if (runq->head != 0) {
      // At least one task was scheduled
      goto run_tasks;
    }
  }

}
