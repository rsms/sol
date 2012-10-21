// Benchmark helpers
#ifndef S_TEST_BENCH_H_
#define S_TEST_BENCH_H_
#include <sol/common.h>

#include <sys/time.h>
#include <sys/resource.h>

// struct rusage {
//   struct timeval ru_utime; /* user time used */
//   struct timeval ru_stime; /* system time used */
//   long ru_maxrss;          /* max resident set size */
//   long ru_ixrss;           /* integral shared text memory size */
//   long ru_idrss;           /* integral unshared data size */
//   long ru_isrss;           /* integral unshared stack size */
//   long ru_minflt;          /* page reclaims */
//   long ru_majflt;          /* page faults */
//   long ru_nswap;           /* swaps */
//   long ru_inblock;         /* block input operations */
//   long ru_oublock;         /* block output operations */
//   long ru_msgsnd;          /* messages sent */
//   long ru_msgrcv;          /* messages received */
//   long ru_nsignals;        /* signals received */
//   long ru_nvcsw;           /* voluntary context switches */
//   long ru_nivcsw;          /* involuntary context switches */
// };

typedef struct {
  struct rusage r;
  struct timeval rtime;
} SResUsage;

inline static bool S_UNUSED SResUsageSample(SResUsage* usage) {
  gettimeofday(&usage->rtime, 0);
  return getrusage(RUSAGE_SELF, &usage->r) == 0;
}

inline static uint64_t S_UNUSED STimevalUSecs(const struct timeval* tv) {
  return ((uint64_t)tv->tv_sec * 1000000ULL) + tv->tv_usec;
}

inline static void S_UNUSED
STimevalDelta(const struct timeval* a,
              const struct timeval *b,
              struct timeval* d) {
  uint64_t d_usec = STimevalUSecs(b) - STimevalUSecs(a);
  d->tv_sec = d_usec / 1000000ULL;
  d->tv_usec = d_usec - (d->tv_sec * 1000000ULL);
}

inline static void S_UNUSED
SResUsageDelta(const SResUsage* a, const SResUsage* b, SResUsage* d) {
  STimevalDelta(&a->rtime, &b->rtime, &d->rtime);
  STimevalDelta(&a->r.ru_utime, &b->r.ru_utime, &d->r.ru_utime);
  STimevalDelta(&a->r.ru_stime, &b->r.ru_stime, &d->r.ru_stime);
  d->r.ru_maxrss =    b->r.ru_maxrss   - a->r.ru_maxrss;
  d->r.ru_ixrss =     b->r.ru_ixrss    - a->r.ru_ixrss;
  d->r.ru_idrss =     b->r.ru_idrss    - a->r.ru_idrss;
  d->r.ru_isrss =     b->r.ru_isrss    - a->r.ru_isrss;
  d->r.ru_minflt =    b->r.ru_minflt   - a->r.ru_minflt;
  d->r.ru_majflt =    b->r.ru_majflt   - a->r.ru_majflt;
  d->r.ru_nswap =     b->r.ru_nswap    - a->r.ru_nswap;
  d->r.ru_inblock =   b->r.ru_inblock  - a->r.ru_inblock;
  d->r.ru_oublock =   b->r.ru_oublock  - a->r.ru_oublock;
  d->r.ru_msgsnd =    b->r.ru_msgsnd   - a->r.ru_msgsnd;
  d->r.ru_msgrcv =    b->r.ru_msgrcv   - a->r.ru_msgrcv;
  d->r.ru_nsignals =  b->r.ru_nsignals - a->r.ru_nsignals;
  d->r.ru_nvcsw =     b->r.ru_nvcsw    - a->r.ru_nvcsw;
  d->r.ru_nivcsw =    b->r.ru_nivcsw   - a->r.ru_nivcsw;
}

inline static void S_UNUSED
SResUsagePrint(const char *leader, const SResUsage *ru, bool is_delta) {
  // From http://man7.org/tlpi/code/online/dist/procres/print_rusage.c.html
  #if !S_TEST_SUIT_RUNNING
  const char *ldr;

  ldr = (leader == NULL) ? "" : leader;

  printf("%sReal time (secs):        %lu.%lu\n", ldr,
         ((unsigned long)ru->rtime.tv_sec),
         ((unsigned long)ru->rtime.tv_usec / 1000UL) );
  printf("%sCPU time (secs):         user: %lu.%lu, system: %lu.%lu\n", ldr,
         ((unsigned long)ru->r.ru_utime.tv_sec),
         ((unsigned long)ru->r.ru_utime.tv_usec / 1000UL),
         ((unsigned long)ru->r.ru_stime.tv_sec),
         ((unsigned long)ru->r.ru_stime.tv_usec / 1000UL) );
  if (is_delta) {
    printf("%sDelta resident set size: %ld\n", ldr, ru->r.ru_maxrss);
  } else {
    printf("%sMax resident set size:   %ld\n", ldr, ru->r.ru_maxrss);
  }
  printf("%sIntegral shared memory:  %ld\n", ldr, ru->r.ru_ixrss);
  printf("%sIntegral unshared data:  %ld\n", ldr, ru->r.ru_idrss);
  printf("%sIntegral unshared stack: %ld\n", ldr, ru->r.ru_isrss);
  printf("%sPage reclaims:           %ld\n", ldr, ru->r.ru_minflt);
  printf("%sPage faults:             %ld\n", ldr, ru->r.ru_majflt);
  printf("%sSwaps:                   %ld\n", ldr, ru->r.ru_nswap);
  printf("%sBlock I/Os:              input=%ld; output=%ld\n",
          ldr, ru->r.ru_inblock, ru->r.ru_oublock);
  printf("%sSignals received:        %ld\n", ldr, ru->r.ru_nsignals);
  printf("%sIPC messages:            sent=%ld; received=%ld\n",
          ldr, ru->r.ru_msgsnd, ru->r.ru_msgrcv);
  printf("%sContext switches:        voluntary=%ld; "
          "involuntary=%ld\n", ldr, ru->r.ru_nvcsw, ru->r.ru_nivcsw);
  #endif
}

inline static void S_UNUSED
SResUsagePrintSummary(const SResUsage *start,
                      const SResUsage *end,
                      const char* opname,
                      size_t opcount,
                      size_t thread_count) {
  #if !S_TEST_SUIT_RUNNING
  const char* _opname = (opname == 0) ? "operation" : opname;
  SResUsage ru_delta;
  SResUsageDelta(start, end, &ru_delta);
  SResUsagePrint("", &ru_delta, true);
  double u_nsperop =
    ((double)STimevalUSecs(&ru_delta.r.ru_utime) / opcount) * 1000.0;
  double r_nsperop =
    ((double)STimevalUSecs(&ru_delta.rtime) / opcount) * 1000.0;
  print("Performance: %.0f ns per %s (CPU user time)", u_nsperop, _opname);
  print("             %.0f ns per %s (real time)", r_nsperop, _opname);
  if (thread_count > 1) {
    print("Overhead:    %.0f ns in total",
      (r_nsperop * thread_count) - u_nsperop );
  }
  print("%s total: %zu", _opname, opcount);
  #endif
}


#endif  // S_TEST_BENCH_H_
