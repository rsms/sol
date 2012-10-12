#include <sol/common.h>
#include <sol/sched.h>
#include <sol/log.h>
#include <sol/host.h>
#include <sol/vm.h>
#include <sol/value.h>

//static const SInstr load_instr1 = SInstr_LOADK(0, 1);
static const SInstr load_instr = S_INSTR_ABu(S_OP_LOADK, 0, 1);

int main(int argc, const char** argv) {
  printf("Sol " S_VERSION_STRING " " S_TARGET_ARCH_NAME "\n");
  SLogD("SHostAvailCPUCount: %u", SHostAvailCPUCount());

  SVM vm = SVM_INIT;

  // A simple program which assigns a value to a variable 'x' and decrements the
  // value of that variable until it is no longer larger than '0'. Each time it
  // decrements a value it yields for other tasks.
  //    ---------------------
  // 0  x = 5
  // 1  while (x > 0)
  // 2    x = x - 1
  // 3    yield
  // 4  return
  //
  //    Translates to -->
  //    ---------------------
  // 0  x = 5
  // 1  if (x <= 0) then (goto 5) else
  // 2    x = x - 1
  // 3    yield
  // 4  goto 1
  // 5  return

  // Constants
  SValue constants[] = {
    SValueNumber(5),
    SValueNumber(0),
    SValueNumber(1),
  };

  // Instructions
  SInstr instructions[] = {
    SInstr_LOADK(0, 0),        // 0  R(0) = K(0)
    SInstr_LE(0, 0, 255+1),    // 1  if (RK(0) <= RK(k+1)) else PC++
    SInstr_JUMP(3),            // 2    PC += 3 to RETURN
    SInstr_SUB(0, 0, 255+2),   // 3    R(0) = R(0) - RK(k+1)
    SInstr_YIELD(0, 0),        // 4    yield 0  ; A=yield cpu, Bu=ignored
    SInstr_JUMP(-5),           // 5    PC -= 5 to LE
    SInstr_RETURN(0, 0),       // 6  return
  };

  SSched* sched = SSchedCreate();

  // Schedule several tasks running the same program
  SSchedTask(sched, STaskCreate(instructions, constants));
  SSchedTask(sched, STaskCreate(instructions, constants));
  // SSchedTask(sched, STaskCreate(instructions, constants));
  
  SSchedRun(&vm, sched);

  SSchedDestroy(sched);
  printf("Scheduler runloop exited.\n");
  return 0;
}
