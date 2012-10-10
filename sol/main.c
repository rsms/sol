#include <sol/common.h>
#include <sol/sched.h>
#include <sol/debug.h>

int main(int argc, const char** argv) {
  SInstr ins;

  ins = S_INSTR_ABC(S_INSTR_OP_MAX, S_INSTR_A_MAX, S_INSTR_B_MAX, S_INSTR_C_MAX);
  printf("instr: %s\n", SDFormatBin(ins, 32));
  printf("   OP: %u, A: %u, B: %u, C: %u\n",
    SInstrGetOP(ins),
    SInstrGetA(ins),
    SInstrGetB(ins),
    SInstrGetC(ins));

  return 0;

  // printf("4:     %s\n", SDFormatBin(4, 32));
  // printf("-4:    %s\n", SDFormatBin((uint32_t)-4, 32));
  // SInstr jump = SInstr_JUMP((uint32_t)-4);
  // SInstr loadk = SInstr_LOADK(1, 2);
  // printf("jump:  %s\n", SDFormatBin(jump, 32));
  // printf("loadk: %s\n", SDFormatBin(loadk, 32));

  return 0;

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
