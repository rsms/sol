// Tests the message queue, which is a lock-free MP,SC FIFO queue.
#include "test.h"
#include "bench.h"
#include <sol/common.h>
#include <sol/host.h> // for SHostAvailCPUCount
#include <sol/msg.h>
#include "../sol/host.c"
#include "../sol/msg.c"

// TODO: Disable this test if the system does not have pthreads
#include <pthread.h>

#if S_TEST_SUIT_RUNNING
#define VALUE_COUNT  10000
#else
#define VALUE_COUNT  10000
//#define VALUE_COUNT  10000000
#endif

size_t thread_count = 3;
uint32_t total_count = 0;

typedef struct Thread {
  uint32_t  tid;
  pthread_t thread;
  SMsgQ*    q;
} Thread;

void* thread_main(void* d) {
  Thread* t = (Thread*)d;
  bool is_consumer = (t->tid == 1);
  uint32_t count;

  if (is_consumer) {
    // We must receive (<number of producers> * VALUE_COUNT) values.
    //sleep(1);
    print("  [consumer] starting");
    uint32_t total_value_count = (thread_count-1) * VALUE_COUNT;

    // Eeeehhh I'm tired. This can probably be done much more elegantly
    SNumber sum = 0;
    SNumber expected_sum = 0;
    count = VALUE_COUNT;
    do { expected_sum += (SNumber)count; } while(--count);
    expected_sum *= (SNumber)(thread_count-1);

    count = total_value_count;
    size_t failures = 0;

    do {
      SMsg* n = SMsgDequeue(t->q);
      if (n) {
        failures = 0;
        --count;
        sum += n->value.value.n;
        //print("  [consumer] recv %u", n->value);
        free(n);
      } else if (++failures == 100000) {
        SAssert(!"Too many consecutive 'dequeue' errors");
      }
    } while (count);
    print("  [consumer] received all %u values (sum: " SNumberFormat ")",
          total_value_count, sum);
    assert(sum == expected_sum);

  } else {
    // We must produce VALUE_COUNT values
    count = VALUE_COUNT;
    print("[producer %u] starting, will send %u values", t->tid, count);
    do {
      //print("[producer %u] sending %u", t->tid, count);
      SMsg* n = (SMsg*)malloc(sizeof(SMsg));
      n->value.value.n = (SNumber)count;
      SMsgEnqueue(t->q, n);
    } while (--count);
    print("[producer %u] exiting", t->tid);
  }

#if !S_WITHOUT_SMP
  pthread_exit(0);
#endif

  return 0;
}

bool spawn_thread(Thread* t) {
  if (pthread_create(&t->thread, 0, &thread_main, (void*)t) != 0) {
    perror("pthread_create");
    return false;
  } else {
    return true;
  }
}

int main() {
  // Use a minimum of <thread_count> OS threads
  thread_count = S_MAX(thread_count, SHostAvailCPUCount());
  total_count = (thread_count-1) * VALUE_COUNT;
  SMsgQ q = S_MSGQ_INIT(q);
  SResUsage rstart;

#if S_WITHOUT_SMP
  thread_count = 2; // or our sum and count calculations will be off
  total_count = VALUE_COUNT;
  Thread producer = {0,0,&q};
  Thread consumer = {1,0,&q};
  SAssertTrue(SResUsageSample(&rstart));
  thread_main((void*)&producer);
  thread_main((void*)&consumer);

#else // S_WITHOUT_SMP
  print("thread_count: %zu", thread_count);
  Thread* threads = (Thread*)malloc(sizeof(Thread) * thread_count);
  SAssertTrue(SResUsageSample(&rstart));

  size_t i = 0;
  for (; i != thread_count; ++i) {
    threads[i].tid = (uint32_t)i;
    threads[i].q = &q;
    if (!spawn_thread(&threads[i])) {
      return 1;
    }
  }

  for (i = 0; i != thread_count; ++i) {
    void* exit_value = 0;
    if (pthread_join(threads[i].thread, &exit_value) != 0) {
      perror("pthread_join");
    }
  }
#endif // S_WITHOUT_SMP

  SAssertNil(SMsgDequeue(&q));

  // Sample #2 and print stats
  #if !S_TEST_SUIT_RUNNING
  SResUsage rend;
  SAssertTrue(SResUsageSample(&rend));
  SResUsagePrintSummary(&rstart, &rend, "send+recv", total_count, thread_count);
  #endif

  pthread_exit(NULL);
  return 0;
}
