// Helpers for tests
#ifndef S_TEST_TEST_H_
#define S_TEST_TEST_H_

#include <sol/common.h>

#if !S_DEBUG
#warning "Running test in release mode (no assertions). Makes no sense."
#endif

#if S_TEST_SUIT_RUNNING
  #define print(...) ((void)0)
#else
  #define print(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#endif

#include <sol/vm.h>
#include <sol/sched.h>
#include <sol/task.h>
#include <sol/instr.h>
#include <sol/debug.h>
#include <sol/log.h>

static void __attribute__((constructor)) init_test() {
  SLogStream = stdout;
}

#endif  // S_TEST_TEST_H_
