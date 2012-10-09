#include <sol/common.h>
#include <sol/sched.h>

int main(int argc, const char** argv) {

  int i = 0;
  SInstr opcodes[128] = { 0, };
  // i32 x = 10
  // while x > 0
  //   x = x - 1
  //   yield
  // return
  opcodes[i++] = SInstr_LOADK(0, 1);       // 0  loadk r0,1
  opcodes[i++] = SInstr_LT(0, 255, 0);     // 1  k0 < r0 ? (skips next if 1)
  opcodes[i++] = SInstr_JUMP(3);           // 2    to 6 (3+3)
  opcodes[i++] = SInstr_SUBI(0, 0, 255+1); // 3    r0 = r0 - k1
  opcodes[i++] = SInstr_YIELD(0, 0);       // 4    yield
  opcodes[i++] = SInstr_JUMP(-5);          // 5    to 1 (6-5)
  opcodes[i++] = SInstr_RETURN(0, 0);      // 6  return

  SSched* sched = SSchedCreate();

  STask* task1 = STaskCreate(opcodes, i);
  STask* task2 = STaskCreate(opcodes, i);
  STask* task3 = STaskCreate(opcodes, i);

  SSchedEnqueue(sched, task1);
  SSchedEnqueue(sched, task2);
  SSchedEnqueue(sched, task3);
  
  SSchedRunLoop(sched);
  SSchedDestroy(sched);

  printf("Scheduler runloop exited.\n");
  return 0;
}
