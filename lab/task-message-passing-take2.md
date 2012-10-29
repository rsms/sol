Erlang snippet re pids

    erts_smp_atomic_inc(&process_count);
    p->id = make_internal_pid(p_serial << p_serial_shift | p_next);
    if (p->id == ERTS_INVALID_PID) {
    	/* Do not use the invalid pid; change serial */
    	p_serial++;
    	p_serial &= p_serial_mask;
    	p->id = make_internal_pid(p_serial << p_serial_shift | p_next);
    	ASSERT(p->id != ERTS_INVALID_PID);
    }
    ASSERT(internal_pid_serial(p->id) <= (erts_use_r9_pids_ports
    					  ? ERTS_MAX_PID_R9_SERIAL
    					  : ERTS_MAX_PID_SERIAL));

## 1. send msg to tid

+ Easily parallelizable (no dangling refs to tasks).
+ Efficient implementation a swe know there's only one consumer.
+ Simple programming, as there's no need for channel creation.
- TIDs must either be secure (that is hard) or any task might send a message to any other task
- Either every task must have an inbox (uses ~100 bytes or memory), or there needs to be a lazy instantiation of an inbox.

    y = fun:
      Connection c = recv
      send "ok" to __parent
    x = fun:
      tid = spawn y
      Connection c = accept
      send c to tid
      Text status = recv


## 2. send msg to T

+ Secure as T is explicitly handed over to non-family tasks.
+ Efficient implementation a swe know there's only one consumer.
+ Simple programming (any task can be sent a message.)
- Hard to parallelize as a reference to a task would be… actually this is equally hard as alternative 3.
- Either every task must have an inbox (uses ~100 bytes or memory), or there needs to be a lazy instantiation of an inbox.

    y = fun:
      Connection c = recv
      send "ok" to __parent
    x = fun:
      t = spawn y
      Connection c = accept
      send c to t
      Text status = recv

## 3 send msg over chan

+ Simpler implementation as a task has no inbox.
+ More flexible as a single channel can be used across many processes. In the case where there's one producer (e.g. "import jobs, preprocess and then have the first available worker take on the job" could be efficiently implemented with a MP/MC queue, but with a SP/MC queue, the producer would need to decide which task to give the job).
- If the common case is for tasks to communicate, this requires boiler-plate code (making a channel, passing it, etc).
- Less effective queue implementation as single-consumer can not be assumed (must assume multiple-consumers).
- More complex queue and task implementation as a task can be waiting for a specific queue (with an inbox model, the queue is implied by the "wait_msg" flag).
- There would be no "cascading errors" of task upon errors, where a subtask sends "error" to its parent when an error occurs and the parent will subsequently propagate that error by exiting and so on.

    y = fun chan:
      Connection c = recv from chan
      send "ok" over chan
    x = fun:
      chan = Chan{}
      t = spawn y with args chan
      Connection c = accept
      send c over chan
      Text status = recv from chan

## Questions

1. Can we efficiently and simplisticly implement a variable-channel listener?
2. Can we implement a MP/MC queue that comes close to the efficiency of the current MP/SC queue?
3. Are we pursuing a mental model of "task hierarchy" or "sea of potentially communicating tasks"?

A "task group" (aka Erlang-style) model might be favorable as it is likely to be easier to understand. We could provide a library function for MP/MC queues ("channel") to solve the use-case of "let the first available subtask receive this message", but retain "cascading errors" and "task hierarchy".

Error handling is important as we assume these systems will inherently deal with errors.

Erlang has two "types" of errors:

- _Exceptions_ occur when the run-time system does not know what to do. E.g. when asked to open a file which does not exist.
- _Errors_ occur when the programmer doesn’t know what to do. I.e. when there's no way to deal with an unexpected result (e.g. status code returned from a subtask is unknown to the receiving task and so the task has no way of dealing with it).

This is a very sensible error model. An _Error_ would cause a task to exit with that error as the return argument. An _Exception_ would be an error that is trapped by user code. Thus, an _Exception_ which is not trapped in a task is an _Error_.

### Alternatives for trapping errors

A:

    f, error = fs:open "bar"
    # do something about err ...

B:

    try:
      f = fs:open "bar"
    catch error:
      # do something about err ...

For any case, if error is not trapped:

    f = fs:open "bar"
    # crash on error

I lean toward alternative A as the high-level protocol could be very simple and easy to understand. BUT! This needs to be universally applicable to any function. How about this case:

    read = fun fn:
      f = fs:open fn
      data = (fs:read f)
      if (len data) > 100:
        return data, "large"
      else:
        return data
    main = fun:
      _, data = read "bar"

#### Hurdle

What happens when `fs:open` fails in the `read` function? The `read` function has a variable number of return values, so we can't simply return prematurely with `nil, nil, error`. Or perhaps we can. We could make a rule that any function in Sol must declare their return types, and so the number of return values are constant.

Our `read` function would be defined as:

    read = fun fn -> Data Text:
      ...

And so when an error occurs (e.g. in `fs:open`) we simply return N number of `nil`s followed by the error, where N is the number of declared return values. Then, the landing code (after a function call) would inspect the number of landing values; R. If R is <= to N, then check if there's an error set. If there is, propagate the error. If R is == N+1, then set landing value R-1 to any error, or `nil` if no error was set.

In this scenario a function that wishes to return variable arguments could be done so with syntactic sugar

    M = fun -> Text… Int:
      return "a", "b", "c", 9
    main = fun:
      names…, y, error = M()
      name1, name2 = M()

In the second case of calling `M`, trapping of errors would not be possible, which is OK as trapping of errors would be possible as illustrated by the first case (`names…`).


## Scheduler load balancing

    volatile SSched* least_loaded_sched

S1:
    runloop:
      exec()...
      for each S in all_schedulers:
        if S.load <= my_load:
          goto runloop
      least_loaded_sched = S1
S2:
    runloop:
      exec()...
      for each S in all_schedulers:
        if S.load <= my_load:
          goto runloop
      least_loaded_sched = S1
