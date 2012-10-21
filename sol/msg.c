#include "msg.h"

// void SMsgQCreate(SMsgQ* q) {
//   q->head = &q->sentinel;
//   q->tail = &q->sentinel;
//   q->sentinel.next = 0;
// }

// Based on https://groups.google.com/d/msg/lock-free/Vd9xuHrLggE/B9-URa3B37MJ

bool SMsgEnqueue(SMsgQ* q, SMsg* m) {
  m->next = 0;
  SMsg* prev = SAtomicSwap(&q->head, m);
  prev->next = m;
  return prev == &q->sentinel;
}

SMsg* SMsgDequeue(SMsgQ* q) {
  SMsg* tail = q->tail;
  SMsg* next = tail->next;

  if (tail == &q->sentinel) {
    if (next == 0) {
      return 0;
    }
    q->tail = next;
    tail = next;
    next = next->next;
  }

  if (next != 0) {
    q->tail = next;
    return tail;
  }

  SMsg* head = q->head;
  if (tail != head) {
    return 0;
  }

  SMsgEnqueue(q, &q->sentinel);
  next = tail->next;

  if (next) {
    q->tail = next;
    return tail;
  }
  return 0;
}
