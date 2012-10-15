#include <sol/common.h>
#include <sol/sched.h>
#include <sol/log.h>
#include <sol/host.h>
#include <sol/vm.h>
#include <sol/func.h>

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
  // SValue constants[] = {
  //   SValueNumber(5),
  //   SValueNumber(0),
  //   SValueNumber(1),
  // };

  // Instructions
  // SInstr instructions[] = {
  //   SInstr_LOADK(0, 0),        // 0  R(0) = K(0)
  //   SInstr_LE(0, 0, 255+1),    // 1  if (RK(0) <= RK(k+1)) else PC++
  //   SInstr_JUMP(3),            // 2    PC += 3 to RETURN
  //   SInstr_SUB(0, 0, 255+2),   // 3    R(0) = R(0) - RK(k+1)
  //   SInstr_YIELD(0, 0, 0),     // 4    yield 0  ; A=yield cpu
  //   SInstr_JUMP(-5),           // 5    PC -= 5 to LE
  //   SInstr_RETURN(0, 0),       // 6  return
  // };
  //
  // // Timeout timer ("sleep") test program.
  // // 0  delay = 1200
  // // 1  sleep(delay)
  // // 2  return
  // SValue constants[] = {
  //   SValueNumber(1200),
  // };
  // SInstr instructions[] = {
  //   SInstr_LOADK(1, 0),    // 0  R(1) = K(0)
  //   SInstr_YIELD(1, 1, 0), // 1  yield timeout (RK(1) = after_ms)
  //   SInstr_RETURN(0, 0),   // 4  return
  // };
  //
  // // Make a function out of the program
  // SFunc* sleepfun = SFuncCreate(constants, instructions);

  // Function calling
  SValue a_constants[] = {
    SValueNumber(123), // return value
  };
  SInstr a_instructions[] = {
    // Arguments: (R(0)=sleep_ms)
    SInstr_DBGREG(0, 0, 0),// debug
    SInstr_YIELD(1, 0, 0), // yield timeout (RK(0) = after_ms)
    SInstr_LOADK(0, 0),    // R(0) = K(0) = 123
    SInstr_RETURN(0, 1),   // <- R(0)..R(0) = R(0) = 123
  };
  SFunc* a_fun = SFuncCreate(a_constants, a_instructions);
  SValue b_constants[] = {
    SValueFunc(a_fun),
    SValueNumber(500), // argument to a_fun
  };
  SInstr b_instructions[] = {
    SInstr_LOADK(0, 0),    // R(0) = K(0) = a_fun
    SInstr_LOADK(1, 1),    // R(1) = K(1) = 500
    //SInstr_DBGREG(0, 1, 1),// debug
    SInstr_CALL(0, 1, 1),  // R(0)..R(0) = R(0)(R(1)..R(1)) = a_fun(R(1))
    SInstr_DBGREG(0, 1, 0),// debug: So we can inspect a_fun's return value
    SInstr_RETURN(0, 0),   // return
  };
  SFunc* b_fun = SFuncCreate(b_constants, b_instructions);

  // Create a scheduler
  SSched* sched = SSchedCreate();

  // Schedule several tasks running the same program
  SSchedTask(sched, STaskCreate(b_fun));
  // SSchedTask(sched, STaskCreate(fun1));
  // SSchedTask(sched, STaskCreate(fun1));
  
  SSchedRun(&vm, sched);

  SFuncDestroy(b_fun);
  SFuncDestroy(a_fun);
  SSchedDestroy(sched);
  printf("Scheduler runloop exited.\n");
  return 0;
}
