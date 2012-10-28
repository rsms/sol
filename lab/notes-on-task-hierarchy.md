## Hierarchical tasks

    A spawns B
      B spawns C and D
        C
        D
    E spawns F
      F
    ...

When B terminates, C and any other tasks spawned by B must also be terminated (`C` and `D` in the above case). When a task is terminated its parent will receive a message `(TaskInfo, tid, Ended)` about the task having terminated.

Common scenario:

    A -> B
    B -> x
    A

1. Task A spawns task B
2. Task A waits for a message from task B
3. Task B produces a message X for A
4. Task B terminates
5. Task A receives message X
6. Task A receives message "task B ended"


### Approach 1:

    Task { Task* parent }

+ Small memory footprint
- Expensive worst-case scenario where the entire tree of tasks need to be traversed

### Approach 1.1:

    TaskRef { Task* task }
    Task { TaskRef* parent }

Before a task is scheduled, the scheduler checks if its parent is still alive (a TaskRef structure is used to enable setting the value of `parent` to NULL). If the parent is NULL, then the task is imemdiately terminated. Question is, how expensive is this to do every time a task is scheduled in? Perhaps this logic is only executed every N per-task schedule-ins? Also note that a `TaskRef` approach cost *two* pointers of memory.

### Approach 2:

    Task { Task* children[] }

+ Good worst-case scenario where a subtree is traversed top-down.
- Requires memory reallocation of `children` for each task spawned.

### Ideas:

Since when a task (e.g. `A` above) terminates, we know that _all_ it's children also needs terminating. Traversing a tree downwards in this case would less efficient that for instance accessing a set of all tasks spanwed by `A`. Perhaps we can find a task identifier scheme that allows this, i.e. identifiers are hierarchical.

A sparse array as a set for active tasks where the index is the task id?

---

What the run queue is the task graph where we have one magical "root" task?

Imagine the following task hierarchy:

    A
      B
        C
        D
      E
    F
      G
      H

B terminates:

    1. A -> B -> C -> D -> E -> F -> G -> H
    2. A -> (B -> C -> D ->) E -> F -> G -> H
    3. A -> E -> F -> G -> H

Problem: How can a task discern from it's children and other tasks? 
Possible solution: A task can have a `tail` member in addition to it's next pointer:

    Task {
      Task* next
      Task* tail
    }

When a task is spawned:

    if parentT.tail != 0:
      newT.next = parentT.tail.next
    else
      newT.next = parentT.next
      parentT.next = newT
    parentT.tail = newT

Eventually:

    A{
      tail = *E
      next = B{
        tail = *D
        next = C{
          next = D{
            next = E{
              next = F{
                tail = *H
                next = G{
                  next = H{
                  }
          ...
    }

    A{
      tail = 0
      next = F{
        next = 0
      }
    }

--> A spawns B -->

    A{
      tail = *B
      next = B{
        next = F{
          next = 0
        }
    }

To save room, the `tail` could be implemented as an offset from `next` rather than an absolute address. In a setupd like this, where `next` and `tail` must not be nil for live tasks, the scheduler can no longer test the `next` member for nil to decide wether to execute a task or not. Instead, we add a member `status` to the task type:

    Task {
      Task* next
      Task* tail
      enum { Running, Suspended } status;
    }

The scheduler will then conditionally execute a task depending on its status:

    SSchedRun(SSched* s) {
      Task* t = s->task_head;
      while (t) {
        if (t->status == Running) {
          SSchedExec(t)
          // check if task ended and if so, unlink etc
        }
        if (t->next == 0) {
          t = s->task_head;
        } else {
          t = t->next;
        }
      }
    }

Problem with this approach: When a task spawns a new task, the new task is scheduled to run _before_ existing tasks. This is dangerous as given a scenario where many tasks spawn new tasks, preexisting tasks might never run!

Let's explore an alternative:

    Task {
      Task* parent;    // Task that owns us
      Task* tail;      // Last task we own
      Task* next;      // Next scheduled task (cyclic)
    }

Scheduler routines:

    SSchedTask(SSched* s, STask* t, STask* pt):
      if s.head == 0:
        s.head = t
      else:
        s.tail.next = t
      s.tail = t
      if (pt != 0):
        t.parent = pt
      pt.tail = t

Runloop:

    prev_task = FindTaskJustBeforeTail() # called seldomly
    t = s.head
    while (t != 0):
      status = SSchedExec(t)
      
      if (status != Yield):
        # Here status is either Suspend or End
        if (s.head == s.tail):
          # t is the only task scheduled
          s.head = 0
          s.tail = 0
        else:
          assert(prev_task.next == t)
          prev_task.next = t.next
          if (t == s.head):
            s.head = t.next
            assert(s.tail.next == t)
            s.tail.next = s.head
          else if (t == s.tail):
            s.tail = prev_task
        
        if (status == End):
          if (t.parent != 0)
            NotifyParentOfSubtaskExit(t, End)
          if (t.tail != 0)
            TerminateAllSubtasks(t)
          STaskDestroy(t)
      
      else:
        prev_task = t
      
      t = t.next

---

Initial state of scheduler `s`: ()

    s.tail = 0
    s.head = 0
    
A is scheduled: (A)

    s.tail = *A
    s.head = A{
      next = *A }

F is scheduled: (A -> F)

    s.tail = *F
    s.head = A{
      next = F{
        next = *A }}

A spawns B: (A -> F -> B)

    s.tail = *B
    s.head = A{
      tail = *B
      next = F{
        next = B{
          parent = *A
          next = *A }}}

A spawns C: (A -> F -> B -> C)

    s.tail = *C
    s.head = A{
      tail = *C
      next = F{
        next = B{
          parent = *A
          next = C{
            parent = *A
            next = *A }}}}

F spawns G: (A -> F -> B -> C -> G)

    s.tail = *G
    s.head = A{
      tail = *C
      next = F{
        tail = *G
        next = B{
          parent = *A
          next = C{
            parent = *A
            next = G{
              parent = *F
              next = *A }}}}}

B is supended (e.g. waiting for a timer):

    s.tail = *G
    s.head = A{
      tail = *C
      next = F{
        tail = *G
        next = C{
          parent = *A
          next = G{
            parent = *F
            next = *A }}}}

B is resumed (e.g. from a timer triggering):

    s.tail = *B
    s.head = A{
      tail = *C
      next = F{
        tail = *G
        next = C{
          parent = *A
          next = G{
            parent = *F
            next = B{
              parent = *A
              next = *A }}}}}

A is suspended:

    s.tail = *B
    s.head = F{
      tail = *G
      next = C{
        parent = *A
        next = G{
          parent = *F
          next = B{
            parent = *A
            next = *F }}}}

A is resumed:

    s.tail = *A
    s.head = F{
      tail = *G
      next = C{
        parent = *A
        next = G{
          parent = *F
          next = B{
            parent = *A
            next = A{
              tail = *C
              next = *F }}}}}

C ends:

    s.tail = *A
    s.head = F{
      tail = *G
      next = G{
        parent = *F
        next = B{
          parent = *A
          next = A{
            tail = *B
            next = *F }}}}

A ends, taking subtasks with it:

    s.tail = *G
    s.head = F{
      tail = *G
      next = G{
        parent = *F
        next = *F }}



- unschedule a task
- suspend a task
- a task with children dies, causing children to die too
- a task with a parent dies, informs the parent



















