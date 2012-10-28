Task message passing. Simple FIFO should do.

    A: B = spawn bfun
    B: x = recv()
    A: "hello" -> B
    B: print x  # "hello"

### Approach 1:

Erlang-style, each task has an inbox.

    Task {
      FIFO inbox
      Flag msg_wait
    }

- Must be a way for a task to wait for input to arrive on it's inbox. We could solve this with a YIELD instruction type "wait-inbox", or simply a SEND and RECV instruction pair with yield similarities, avoiding higher-level locking or semaphores.

    task A> SPAWN ...
    sched>  runqueue_push(new_task(...))
    task B> RECV 0 1 0
    sched>  if (T->inbox is empty):
              T->msg_wait = 1
              runqueue_pop(T)
    task A> MOVEK 0 0  ; R(0) = <tid B>
    task A> MOVEK 1 1  ; R(1) = "hello"
    task A> SEND 0 1 1
    sched>  T = find_task(R(A)) = <task B>
    sched>  inbox_push(T->inbox, R(B))
    sched>  if (T->msg_wait):
              runqueue_push(T)
    task B> CALL … print R(0) ; "hello" is in R(0)
    
    