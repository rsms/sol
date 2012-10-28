## Summary

- A task has an inbox (MP/SC queue)
- When sending messages, tasks are referenced by themselves (not a "TID")
- A future library function might provide a "channel" (MP/MS queue) for cases where multiple consumers is desired
- A task maintains a reference to its supertask (which spawned it)
- When tasks are spawned, arguments can be passed to the entry function
- Errors cascade — when a subtask end-with-error, the supertask propagates that error, unless the supertask is trapping subtask erros
- Tasks can set a flag to trap subprocess end-with-errors, e.g. to perform clean up or restart subtasks
- Functions can return multiple results and an untrapped error is always returned as the last+1 result
- As an incremental improvement, schedulers could perform "zombie task collection" when idle or lightly loaded by traversing all suspended tasks to see if they are waiting for a message from a dead supertask (and if so cause them to end with an error "task dead" or "task inbox closed"). That should be sufficient to avoid fatal leaks.