// Tests the Task ID map, a data structure that maps a tasks by their `tid`s.
#include <sol/common.h>
#include <sol/tidmap.h>
#include "../sol/tidmap.c"

const size_t taskc = 6;
STask taskv[taskc];

bool test_add_get() {
  STIDMap tidmap = S_TID_MAP_INIT;

  // Add each task
  for (size_t i = 0; i != taskc; ++i) {
    bool added = STIDMapAdd(&tidmap, &taskv[i]);
    assert(added == true);
    //printf("put %u -> %p\n", taskv[i].tid, &taskv[i]);
  }

  // Get each task added
  for (size_t i = 0; i != taskc; ++i) {
    STask* t = STIDMapGet(&tidmap, taskv[i].tid);
    //printf("get %u => %p (expected %p)\n", taskv[i].tid, t, &taskv[i]);
    assert(t == &taskv[i]);
  }

  // Remove each task added
  for (size_t i = 0; i != taskc; ++i) {
    STask* t = STIDMapRemove(&tidmap, taskv[i].tid);
    //printf("get %u => %p (expected %p)\n", taskv[i].tid, t, &taskv[i]);
    assert(t == &taskv[i]);
  }

  return true;
}

int main() {
  // Initialize test data
  memset(taskv, 0, sizeof(taskv));
  taskv[0].tid = 5;
  taskv[1].tid = 9174;
  taskv[2].tid = 191939710;
  taskv[3].tid = 0;
  taskv[4].tid = STaskIDMax;
  taskv[5].tid = 201;

  if (!test_add_get()) return 1;

  return 0;
}
