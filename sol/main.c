#include <sol/common.h>
#include <sol/sched.h>
#include <sol/log.h>
#include <sol/host.h>

//static const SInstr load_instr1 = SInstr_LOADK(0, 1);
static const SInstr load_instr = S_INSTR_ABu(S_OP_LOADK, 0, 1);

int main(int argc, const char** argv) {
  printf("Sol " S_VERSION_STRING " " S_TARGET_ARCH_NAME "\n");
  SLogD("SHostAvailCPUCount: %u", SHostAvailCPUCount());

  // i32 x = 10
  // while x > 0
  //   x = x - 1
  //   yield
  // return
  SInstr opcodes[] = {
    SInstr_LOADK(0, 1),       // 0  loadk r0,1
    SInstr_LT(0, 255, 0),     // 1  k0 < r0 ? (skips next if 1)
    SInstr_JUMP(3),           // 2    to 6 (3+3)
    SInstr_SUBI(0, 0, 255+1), // 3    r0 = r0 - k1
    SInstr_YIELD(0, 0),       // 4    yield 0  ; A=yield cpu, Bu=ignored
    SInstr_JUMP(-5),          // 5    to 1 (6-5)
    SInstr_RETURN(0, 0),      // 6  return
  };

  SSched* sched = SSchedCreate();

  // Schedule several tasks running the same program
  SSchedTask(sched, STaskCreate(opcodes, S_countof(opcodes)));
  SSchedTask(sched, STaskCreate(opcodes, S_countof(opcodes)));
  SSchedTask(sched, STaskCreate(opcodes, S_countof(opcodes)));
  
  SSchedRun(sched);

  SSchedDestroy(sched);
  printf("Scheduler runloop exited.\n");
  return 0;
}
