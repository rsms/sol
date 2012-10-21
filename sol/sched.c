#include "sched.h"
#include "instr.h"
#include "log.h"
#include "../deps/libev/ev.h"

typedef struct ev_loop EVLoop;

#if S_DEBUG
void _DumpQ(STask* t) {
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
}
void _DumpRQAndWQ(SSched* s) {
  printf("[sched %p] run queue:", s);
  _DumpQ(s->rhead);
  printf("\n[sched %p] wait queue:", s);
  _DumpQ(s->whead);
  printf("\n");
}
#else
#define _DumpRQAndWQ(x) ((void)0)
#endif

SSched* SSchedCreate() {
  SSched* s = (SSched*)malloc(sizeof(SSched));

  // Initialize run queue
  s->rhead = 0;
  s->rtail = 0;

  // Initialize waiting queue
  s->whead = 0;
  s->wtail = 0;

  // Create a new ev_loop
  s->events_ = (void*)ev_loop_new(EVFLAG_AUTO | EVFLAG_NOENV);

  // Set the scheduler as "userdata" of the evloop so that ev callbacks can
  // access the owning scheduler.
  ev_set_userdata((EVLoop*)s->events_, (void*)s);

  return s;
}

void SSchedDestroy(SSched* s) {
  // TODO: Free any tasks in RQ and WQ
  ev_loop_destroy((EVLoop*)s->events_);
  free((void*)s);
}

// Add `t` to the end of a doubly-linked list.
inline static void S_ALWAYS_INLINE
_ListPush(STask** head, STask** tail, STask* t) {
  t->next = 0; // or we will mess up the tail
  if (*head == 0) {
    assert(*tail == 0); // head and tail must be empty or not empty together
    *head = t;
  } else {
    (*tail)->next = t;
  }
  t->prev = *tail;
  *tail = t;
}

// Remove `t` from a doubly-linked list. t next and prev are NOT zeroed.
inline static void S_ALWAYS_INLINE
_ListRemove(STask** head, STask** tail, STask* t) {
  if (t == *head) {
    assert(t->prev == 0); // as `t` is the head
    *head = t->next;
    if (t == *tail) {
      assert(t->next == 0); // as `t` is the tail
      *tail = 0;
    } else {
      t->next->prev = 0;
    }
  } else {
    assert(t->prev != 0); // as `t` is not the head
    if (t == *tail) {
      assert(t->next == 0); // as `t` is the tail
      *tail = t->prev;
      t->prev->next = 0;
    } else {
      assert(t->next != 0); // as `t` is not the tail
      t->prev->next = t->next;
      t->next->prev = t->prev;
    }
  }
}

// Add a task to the end of the Run Queue
inline static void S_ALWAYS_INLINE _RQPush(SSched* s, STask* t) {
  _ListPush(&s->rhead, &s->rtail, t);
}

inline static void S_ALWAYS_INLINE _RQRemove(SSched* s, STask* t) {
  _ListRemove(&s->rhead, &s->rtail, t);
}

// Add a task to the end of the Suspend Queue
inline static void S_ALWAYS_INLINE _WQPush(SSched* s, STask* t) {
  assert(t->wp != 0); // task must be waiting for something to be in the WQ
  _ListPush(&s->whead, &s->wtail, t);
}

inline static void S_ALWAYS_INLINE _WQRemove(SSched* s, STask* t) {
  _ListRemove(&s->whead, &s->wtail, t);
}

void SSchedTask(SSched* s, STask* t) {
  _RQPush(s, t);
}

typedef struct {
  ev_timer evtimer; // must be head
  STask*   task;
} STimer;

static void _TimerCallback(EVLoop *evloop, ev_timer *w, int revents) {
  SSched* s = (SSched*)ev_userdata(evloop);
  STimer* timer = (STimer*)w;
  _DumpRQAndWQ(s);
  SLogD("[ev] timer triggered -- moving task from WQ to RQ");

  bool sched_is_waiting = (s->rhead == 0 && s->whead != 0);

  // Remove task from wait queue and add it to the run queue
  _WQRemove(s, timer->task);
  _RQPush(s, timer->task);

  // Here, we could have some logic to schedule the task according to some
  // priority level. Right now, we are always scheduling the task at the end of
  // the RQ. This means that events are processed in the order they arrive in.

  free(timer); // FIXME

  // If the scheduler is in the waiting loop, break the ev loop
  if (sched_is_waiting) {
    SLogD("breaking ev loop");
    ev_break(evloop, EVBREAK_ALL);
  }
}

// Start a timer
static inline STimer*
SSchedTimerStart(SSched* s, STask* task, SNumber after_ms, SNumber repeat_ms) {
  STimer* timer = (STimer*)malloc(sizeof(STimer)); // FIXME: malloc
  timer->task = task;
  
  // Set the timer as the tasks "waiting for"
  task->wp = timer;
  task->wtype = STaskWaitTimer;

  // Configure and schedule the timer
  ev_tstamp after_sec =
    (ev_tstamp)(after_ms == (SNumber)0) ? 0 : (after_ms / (SNumber)1000.0);
  
  ev_tstamp repeat_sec =
    (ev_tstamp)(repeat_ms == (SNumber)0) ? 0 : (repeat_ms / (SNumber)1000.0);

  ev_timer_init((ev_timer*)timer, _TimerCallback, after_sec, repeat_sec);
  ev_timer_start((EVLoop*)s->events_, (ev_timer*)timer);
  SLogD("[ev] timer scheduled to trigger after " SNumberFormat " ms", after_ms);

  return timer;
}

static inline void _TimerCancel(SSched* s, STimer* timer) {
  STrace();
  ev_timer_stop((EVLoop*)s->events_, (ev_timer*)timer);
}


// Called when a task ended. Cleans it up and potentially free's it.
// Returns true if `t` has subtasks which are still alive.
inline static bool S_ALWAYS_INLINE
_EndTask(STask* t, STaskStatus status) {
  STrace();

  if (status == STaskStatusError) {
    SLogE("Task error");
  }

  if (t->supt != 0) {
    // This task has a supertask
    SLogD(">>> subtask with supertask");

    if (t->supt->next == &STaskDead) {
      // Supertask is dead
      SLogD(">>> supertask is dead and we might become a zombie");

      // Release our reference to the dead supertask
      // if (!STaskRelease(t->supt)) {
      //   // In this case:
      //   // - Supertask is dead
      //   // - Supertask has more references (more live subtasks)
      //   // If we add "zombie collection" to the scheduler, we could set a flag
      //   // here to speed up a collection as we know there are more zombies.
      //   SLogD(">>> there are more zombies out there");
      // }
    } else {
      // The supertask is still alive and well.

      // TODO: Enqueue a "subtask exited" in parent tasks' inbox
    }

    // Release our reference to the supertask.
    if (STaskRelease(t->supt)) {
      SLogD(">>> we caused the final collection of our supertask");
    }
  }

  // TODO: Free as much as possible from the task. Actually, free everything
  // that needs free'ing except `next` (which is needed for the "zombie" flag).
  // The most important thing is to (optionally first unwind and then) free the
  // AR stack.
  if (t->ar) {
    SARecDestroy(t->ar);
    t->ar = 0;
  }

  // Release our one "live" reference.
  if (STaskRelease(t)) {
    SLogD(">>> task finally collected");
    return false;
  } else {
    SLogD(">>> supertask died before all its subtasks died");
    // There are more references to us. Mark ourselves as undead by setting
    // `next` to `STaskDead`.

    // The following STORE is SMP-safe. However we need to be cautious:
    //
    // - `t` is no longer in any RQ or WQ and so `next` not `prev` will be
    //   accessed by any scheduler. All good.
    //
    // - But if a subtask ends at the same time in a different scheduler,
    //   `_EndTask` will LOAD the value of `next` to compare it against
    //   `STaskDead`. Since we do allow zombies, this race condition can be
    //   ignored.
    //
    // So instead of CAS-ing around the LOAD, which happens _every time_ a task
    // ends which has, or did have, a supertask (basically all tasks). But this
    // case of a supertask dying before all of its subtasks happens more rarely.
    // The `next` value is marked "volatile" and so LOADs should be ordered. We
    // don't need care about CAS here when STORE-ing, since the only value ever
    // stored is this one value. If we ever store anything else into `next`
    // after the task has ended, we will need SAtomicSwap here.
    t->next = (STask*)&STaskDead;

    return true;
  }
}

// For the sake of readability and structure, the `SSchedExec` function is
// defined in a separate file
#include "sched_exec.h"

void SSchedRun(SVM* vm, SSched* s) {
  STask* t;

  EVLoop* evloop = (EVLoop*)s->events_;
  int* evrefs = ev_refcount(evloop);

  _DumpRQAndWQ(s);

  exec_loop:
  t = s->rhead;
  while (t != 0) {

    // If vm instructions are being logged, write a header
    #if S_VM_DEBUG_LOG
    printf(
        "[vm] ______________ ______________ __________ _______ ____ ______________\n"
        "[vm] Task           Function       PC         Op      Values\n"
    );// [vm] 0x7fd431c03930 1          [10] LT      ABC:   0, 255,   0
    #endif

    // Execute the task
    STaskStatus status = SSchedExec(vm, s, t);

    // TODO: Clean this up with a switch
    if (status == STaskStatusError ||
        status == STaskStatusEnd ||
        status == STaskStatusSuspend) {

      // Remove task from the run queue
      _RQRemove(s, t);

      // Store next task in a temporary value
      STask* next_task = t->next;

      if (status == STaskStatusSuspend) {
        // Add task to waiting queue
        _WQPush(s, t);
      } else {
        // Task ended
        _EndTask(t, status);
      }
      
      // Advance to the next task
      t = next_task;

    } else {
      assert(status == STaskStatusYield);
      // Keep task in run queue.
      // Advance to the next task
      t = t->next;
    }

    // `t` could be the special `STaskDead` value if we made a programming error
    // and assigned `STaskDead` to a task that was still in the RQ or WQ.
    assert(t != &STaskDead);

    if (t == 0) {
      // We reached the end of one run queue cycle.
      _DumpRQAndWQ(s);

      // Wrap around the run queue
      t = s->rhead;
    }

    if (t != 0 && *evrefs > 0) {
      // Handle any immediate events that triggered event watchers
      SLogD("[RL] ev_run(NOWAIT) (%d refs)", *evrefs);
      ev_run(evloop, EVRUN_NOWAIT);
    }

  } // while there are queued tasks

  // While there are active event watchers:
  if (*evrefs > 0) {
    SLogD("[EL] ev_run(WAIT) (%d refs)", *evrefs);
    ev_run(evloop, 0);
    _DumpRQAndWQ(s);

    if (s->rhead != 0) {
      // At least one task was scheduled
      goto exec_loop;
    } // else: we fall through and cause the scheduler to exit
  }

  return;
}
