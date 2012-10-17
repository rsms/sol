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

// For external use
void SSchedTask(SSched* s, STask* t) {
  t->parent = (STask*)&STaskRoot;
  _RQPush(s, t);
}

typedef struct {
  ev_timer evtimer; // must be head
  STask* task;
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

static inline void S_ALWAYS_INLINE _TimerCancel(SSched* s, STimer* timer) {
  STrace();
  ev_timer_stop((EVLoop*)s->events_, (ev_timer*)timer);
}


// Cleans up `t`, potentially freeing it. Returns true if there are any live
// subtasks or if a "zombie" parent has more subtasks. Basically, this returns
// true if there are more tasks that need to be checked for zombie parents.
inline static bool S_ALWAYS_INLINE
_TaskPostMortem(STask* t) {
  STrace();
  bool has_live_dependants = false;

  if (t->parent != &STaskRoot) {
    SLogD(">>> has parent");
    // This is a subtask. Release our reference to our parent task.
    if (t->parent->next == &STaskZombie) {
      SLogD(">>> parent is zombie");
      // Parent is deaaaaaddd but alliiiiiveeee at the same time.
      // Release our reference to the undead supertask.
      if (!STaskRelease(t->parent)) {
        // Darn. We weren't the only live sutask of the undead supertask. Set
        // `has_live_dependants` to true to flag "there are more tasks to kill".
        // Remember that `t->parent->refc` is the number of live subtasks, thus
        // since we are in this branch, that means:
        // - t->parent is dead
        // - There are live subtasks of t->parent that need killing
        has_live_dependants = true;
      }
    } else {
      // The supertask is still alive and well.
      // TODO: Send a "subtask ended" message to the parent task
      // Release our reference to the supertask.
      STaskRelease(t->parent);
    }
  }

  // TODO: Free as much as possible from the task. Actually, free everything
  // that needs free'ing except `next` (which is needed for the "zombie" flag).
  // The most important thing is to (optionally first unwind and then) free the
  // AR stack.

  // Release our one "live" reference.
  if (!STaskRelease(t)) {
    SLogD(">>> has children");
    // There are more references to us. Mark ourselves as undead by setting
    // `next` to `STaskZombie`.
    // Note: This is safe as tasks are never shared across OS threads.
    t->next = (STask*)&STaskZombie;
    t->prev = 0;
    // Yup, there are some tasks to kill as effect of this (t's subtasks).
    has_live_dependants = true;
  }

  return has_live_dependants;
}

// Checks all waiting tasks to see if any of them is a zombie (has a supertask
// which is dead ...but still aaaliiiive!)
inline static void S_ALWAYS_INLINE
_CheckZombies(SSched* s, bool* something_died) {
  STrace();
  assert(s->whead != 0); // or it makes no sense being here

  // Clear the `something_died` flag
  *something_died = false;

  // For each task in WQ...
  STask* t = s->whead;

  while (t != 0) {
    if (t->parent->next == &STaskZombie) {
      // Woops. Parent task has died. Kill this subtask.
      SLogD("[ZL] parent died -- letting waiting subtask die too");

      // Cancel whatever events `t` is waiting for, or else events will trigger
      // after `t` has died, which will crash all the things.
      assert(t->wp != 0);
      assert(t->wtype == STaskWaitTimer); // only one type at the moment
      _TimerCancel(s, (STimer*)t->wp);

      // Remove task from WQ
      _DumpRQAndWQ(s);
      _WQRemove(s, t);

      // Clean up
      if (_TaskPostMortem(t)) {
        // Set the `something_died` flag to true. This will cause another
        // visitation to this subroutine, avoiding the case of leaving orphans
        // in the waiting queue.
        *something_died = true;
        // Discussion:
        //
        //  A spawns B
        //  B spawns C
        //  C waits for a message from B, so it's put in the waiting queue
        //  B waits for a message from A, so it's put in the waiting queue
        //  A dies and the run queue loop sets `something_died` to true.
        //
        //   When the run queue ends one cycle, it sees that something died
        //   and invokes the `_CheckZombies` subroutine. `_CheckZombies` finds
        //   that B is a zombie (since it's parent A has died) and so it kills
        //   B.
        //
        //   Now, imagine if we didn't set `something_died` to true here, then
        //   when we exit back to the run queue we might never again call
        //   `_CheckZombies` (in the case no more tasks die).
        //
        //   So when we set this flag to true here, a full run queue cycle
        //   will happen after we return, but after that `_CheckZombies` is
        //   invoked again, which will find that C is a zombie (since it's
        //   parent B has died).
        //
      }
    }

    // Advance to next task in WQ
    t = t->next;

  } // while: for each task in WQ
}

// For the sake of readability and structure, the `SSchedExec` function is
// defined in a separate file
#include "sched_exec.h"

void SSchedRun(SVM* vm, SSched* s) {
  STask* t;

  // Death flag. Keeps track of if any tasks dies in one run queue cycle
  bool something_died = false;

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

    STaskStatus status;

    if (something_died && t->parent->next == &STaskZombie) {
      // Woops. Parent task has died. Let this task die too.
      SLogD("[RL] parent died -- letting running subtask die too.");
      status = STaskStatusEnd;
    } else {
      // Execute the task
      status = SSchedExec(vm, s, t);
    }

    // TODO: Clean this up
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
        // Task died (naturally or from an error)
        if (status == STaskStatusError) {
          SLogE("Task error");
        }
        SLogD(">>>>>>>>");
        // Clean up the task. Returns true if the task has live subtasks.
        if (_TaskPostMortem(t)) {
          // There are more references than one, which means this task has
          // subtasks and we must mark `something_died` so that we can--after
          // this run queue cycle completes--check for zombies in the waiting
          // queue.
          SLogD("[RL] a supertask died before its subtasks."
                " Set something_died = true");
          something_died = true;
        }
      }
      
      // Advance to the next task
      t = next_task;

    } else {
      assert(status == STaskStatusYield);
      // Keep task in run queue
      t = t->next;
    }

    if (t == 0) {
      // We reached the end of one run queue cycle.
      _DumpRQAndWQ(s);

      // Wrap around the run queue
      t = s->rhead;

      if (something_died) {
        // At least one task died this cycle and...
        if (s->whead != 0) {
          // ...there are waiting tasks (which are not in the run queue,
          // naturally), so lets check for zombies among tasks in the waiting
          // queue.

          // `_CheckZombies` will kill any waiting tasks which parent task has
          // died. It will set `something_died` depending on if more checks are
          // needed (i.e. to inspect tasks in the run queue).
          _CheckZombies(s, &something_died);
        } else {
          // Well, 
          something_died = false;
        }
      }
    } else if (*evrefs > 0) {
      // Handle any immediate events that triggered event watchers.
      // Conditional: Only if there are evrefs
      SLogD("[RL] ev_run(NOWAIT) (%d refs)", *evrefs);
      ev_run(evloop, EVRUN_NOWAIT);
    }

  } // while there are queued tasks

  // While there are active event watchers
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
