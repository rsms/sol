// Multiple Producer, Single Consumer lock-free (well, one CAS per "enqueue")
// message queue. Zero or more task running in any scheduler can send a message
// to any other task's inbox, which is a `SMsgQ`.
#ifndef S_MSG_H_
#define S_MSG_H_
#include <sol/common.h>
#include <sol/value.h>

struct STask;

typedef struct SMsg {
  struct SMsg* volatile next;
  SValue                value;
  struct STask*         sender;
} SMsg; // 32

typedef struct SMsgQ {
  SMsg* volatile        head;
  uint8_t               _pad;     // cache line hack
  SMsg*                 tail;
  SMsg                  sentinel;
} SMsgQ; // 56

// Constant initializer. E.g. `q = S_MSGQ_INIT(q);`
#define S_MSGQ_INIT(q) (SMsgQ){&(q).sentinel, 0, &(q).sentinel, {0}}

// Put message `m` at end of queue `q`. Returns true if the queue was empty.
bool SMsgEnqueue(SMsgQ* q, SMsg* m);

// Get the message at the beginning of the queue. Returns 0 if there are no
// messages.
SMsg* SMsgDequeue(SMsgQ* q);

#endif // S_MSG_H_
