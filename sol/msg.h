#ifndef S_MSG_H_
#define S_MSG_H_
#include <sol/common.h>
#include <sol/value.h>

// Multiple Producer, Single Consumer lock-free (well, one CAS per "enqueue")
// message queue. Zero or more task running in any scheduler can send a message
// to any other task's inbox, which is a `SMsgQ`.
// Based on https://groups.google.com/d/msg/lock-free/Vd9xuHrLggE/B9-URa3B37MJ
typedef struct SMsg {
  struct SMsg* volatile next;
  SValue                value;
} SMsg;
// Size is 24 bytes (8-byte aligned with 8-byte pointers)

typedef struct SMsgQ {
  SMsg* volatile        head;
  uint8_t               _pad;     // cache line hack
  SMsg*                 tail;
  SMsg                  sentinel;
} SMsgQ;
// Size is 48 bytes (8-byte aligned with 8-byte pointers)

#define S_MSGQ_INIT(q) {&(q).sentinel, &(q).sentinel, {0}}

void SMsgEnqueue(SMsgQ* q, SMsg* m);
SMsg* SMsgDequeue(SMsgQ* q);

#endif // S_MSG_H_
