// Tests basic programming
#include "test.h"
#include <sol/vm.h>
#include <sol/sched.h>

/*
clang -Wall -g -std=c99 -arch x86_64 -O0 -DS_DEBUG=1 -I.. \
  -DS_TEST_SUIT_RUNNING=1 -L../build/debug/lib -lev -lsol -lpthread \
  -o ../build/test/test_prog_basics.c-smp test_prog_basics.c \
  && lldb ../build/test/test_prog_basics.c-smp
*/

#define SAssertRegType(ri, expected_type) do { \
  SValue* registry = task->ar->registry; \
  if (registry[(ri)].type != SValueT##expected_type) { \
    SLogD("R(%u) type = %d", ri, registry[(ri)].type); \
  } \
  assert(registry[(ri)].type == SValueT##expected_type); \
} while(0)

#define SAssertRegNumVal(ri, expected_val) do { \
  SValue* registry = task->ar->registry; \
  SAssertRegType((ri), Number); \
  if (registry[(ri)].value.n != expected_val) { \
    SLogD("R(%u) = %f", ri, registry[(ri)].value.n); \
  } \
  assert(registry[(ri)].value.n == expected_val); \
} while(0)

void test_data(SVM* vm) {
  // LOADK, MOVE
  SValue constants[] = {
    SValueNumber(5),
    SValueNumber(10),
  };
  SInstr instructions[100] = {
    SInstr_LOADK(0, 1),               // R(0) = K(1) = 10
    SInstr_LOADK(1, 0),               // R(1) = K(0) = 5
    SInstr_YIELD(0, 0, 0),
  };
  size_t instr_offs = 3;
  SInstr* start_pc = instructions + instr_offs - 1;
  SFunc* func = SFuncCreate(constants, instructions);
  SSched* sched = SSchedCreate();
  STask* task = STaskCreate(func, 0, 0);
  
  // R(0) = K(1) == 10.0, R(1) = K(0) == 5.0
  assert(SchedExec(vm, sched, task) == STaskStatusYield);
  SAssertRegNumVal(0, 10.0);
  SAssertRegNumVal(1, 5.0);

  // R(2) = R(0) == 10.0
  task->ar->pc = start_pc;
  instructions[instr_offs]   = SInstr_MOVE(2, 0);
  instructions[instr_offs+1] = SInstr_YIELD(0, 0, 0);
  assert(SchedExec(vm, sched, task) == STaskStatusYield);
  SAssertRegNumVal(2, 10.0);

  SSchedDestroy(sched);
  STaskRelease(task);
  SFuncDestroy(func);
}


void test_arithmetic(SVM* vm) {
  // ADD, SUB, MUL, DIV
  SValue constants[] = {
    SValueNumber(5),
    SValueNumber(10),
  };
  SInstr instructions[100] = {
    SInstr_LOADK(0, 0),               // R(0) = K(0) = 5
    SInstr_LOADK(1, 1),               // R(1) = K(1) = 10
    SInstr_YIELD(0, 0, 0),
  };
  size_t instr_offs = 3;
  SInstr* start_pc = instructions + instr_offs - 1;
  SFunc* func = SFuncCreate(constants, instructions);
  SSched* sched = SSchedCreate();
  STask* task = STaskCreate(func, 0, 0);

  // Load Ks to Rs
  assert(SchedExec(vm, sched, task) == STaskStatusYield);

  // R(2) = R(0) + RK(1) = 15
  task->ar->pc = start_pc;
  instructions[instr_offs]   = SInstr_ADD(2, 0, 1);
  instructions[instr_offs+1] = SInstr_YIELD(0, 0, 0);
  assert(SchedExec(vm, sched, task) == STaskStatusYield);
  SAssertRegNumVal(0, 5.0);
  SAssertRegNumVal(1, 10.0);
  SAssertRegNumVal(2, 15.0);

  // R(2) = R(0) - RK(1) = -5
  task->ar->pc = start_pc;
  instructions[instr_offs]   = SInstr_SUB(2, 0, 1);
  instructions[instr_offs+1] = SInstr_YIELD(0, 0, 0);
  assert(SchedExec(vm, sched, task) == STaskStatusYield);
  SAssertRegNumVal(0, 5.0);
  SAssertRegNumVal(1, 10.0);
  SAssertRegNumVal(2, -5.0);

  // R(2) = R(0) * RK(1) = 15
  task->ar->pc = start_pc;
  instructions[instr_offs]   = SInstr_MUL(2, 0, 1);
  instructions[instr_offs+1] = SInstr_YIELD(0, 0, 0);
  assert(SchedExec(vm, sched, task) == STaskStatusYield);
  SAssertRegNumVal(0, 5.0);
  SAssertRegNumVal(1, 10.0);
  SAssertRegNumVal(2, 50.0);

  // R(2) = R(0) / RK(1) = 15
  task->ar->pc = start_pc;
  instructions[instr_offs]   = SInstr_DIV(2, 0, 1);
  instructions[instr_offs+1] = SInstr_YIELD(0, 0, 0);
  assert(SchedExec(vm, sched, task) == STaskStatusYield);
  SAssertRegNumVal(0, 5.0);
  SAssertRegNumVal(1, 10.0);
  SAssertRegNumVal(2, 0.5);

  SSchedDestroy(sched);
  STaskRelease(task);
  SFuncDestroy(func);
}


void test_logic_tests(SVM* vm) {
  // NOT, EQ, LT, LE, JUMP
  // Note: This test also inherently involves some level of control flow as a
  // test is always followed by a JUMP.
  SValue constants[] = {
    SValueNumber(5),
    SValueNumber(0),
  };
  SInstr instructions[100] = {
    SInstr_LOADK(0, 0),               // R(0) = K(0) = 5
    SInstr_LOADK(1, 1),               // R(1) = K(1) = 0
    SInstr_YIELD(0, 0, 0),
  };
  size_t instr_offs = 3;
  SInstr* start_pc = instructions + instr_offs - 1;
  SFunc* func = SFuncCreate(constants, instructions);
  SSched* sched = SSchedCreate();
  STask* task = STaskCreate(func, 0, 0);
  
  // Load Ks to Rs
  assert(SchedExec(vm, sched, task) == STaskStatusYield);

  // not(5) = false
  task->ar->pc = start_pc;
  instructions[instr_offs]   = SInstr_NOT(2, 0);
  instructions[instr_offs+1] = SInstr_YIELD(0, 0, 0);
  assert(SchedExec(vm, sched, task) == STaskStatusYield);
  SAssertRegNumVal(0, 5.0);
  SAssertRegNumVal(1, 0.0);
  SAssertRegType(2, False); // !5 == 0

  // not(0) = true
  task->ar->pc = start_pc;
  instructions[instr_offs]   = SInstr_NOT(2, 1);
  instructions[instr_offs+1] = SInstr_YIELD(0, 0, 0);
  assert(SchedExec(vm, sched, task) == STaskStatusYield);
  SAssertRegNumVal(0, 5.0);
  SAssertRegNumVal(1, 0.0);
  SAssertRegType(2, True); // !0 == 1

  // 5 == 0 == false
  assert(task->ar->registry[1].value.n != constants[0].value.n);
  task->ar->pc = start_pc;
  instructions[instr_offs]   = SInstr_EQ(0, 0, 1); // 5 == 0
  instructions[instr_offs+1] = SInstr_JUMP(1);
  instructions[instr_offs+2] = SInstr_YIELD(0, 0, 0); // if test failed
  instructions[instr_offs+3] = SInstr_YIELD(0, 0, 0); // if test succeeded
  assert(SchedExec(vm, sched, task) == STaskStatusYield);
  assert(task->ar->pc == start_pc+3); // test failed

  // 5 == 5 == true
  task->ar->registry[1].value.n = constants[0].value.n; // ghetto LOADK 1,0
  assert(task->ar->registry[1].value.n == constants[0].value.n);
  task->ar->pc = start_pc;
  instructions[instr_offs]   = SInstr_EQ(0, 0, 1); // 5 == 5
  instructions[instr_offs+1] = SInstr_JUMP(1);
  instructions[instr_offs+2] = SInstr_YIELD(0, 0, 0); // if test failed
  instructions[instr_offs+3] = SInstr_YIELD(0, 0, 0); // if test succeeded
  assert(SchedExec(vm, sched, task) == STaskStatusYield);
  assert(task->ar->pc == start_pc+4); // test succeeded

  // 5 < 4 == false
  assert(task->ar->registry[0].value.n == 5.0);
  task->ar->registry[1].value.n = 4.0;
  task->ar->pc = start_pc;
  instructions[instr_offs]   = SInstr_LT(0, 0, 1); // 5 < 4
  instructions[instr_offs+1] = SInstr_JUMP(1);
  instructions[instr_offs+2] = SInstr_YIELD(0, 0, 0); // if test failed
  instructions[instr_offs+3] = SInstr_YIELD(0, 0, 0); // if test succeeded
  assert(SchedExec(vm, sched, task) == STaskStatusYield);
  assert(task->ar->pc == start_pc+3); // test failed

  // 5 < 5 == false
  assert(task->ar->registry[0].value.n == 5.0);
  task->ar->registry[1].value.n = 5.0;
  task->ar->pc = start_pc;
  instructions[instr_offs]   = SInstr_LT(0, 0, 1); // 5 < 5
  instructions[instr_offs+1] = SInstr_JUMP(1);
  instructions[instr_offs+2] = SInstr_YIELD(0, 0, 0); // if test failed
  instructions[instr_offs+3] = SInstr_YIELD(0, 0, 0); // if test succeeded
  assert(SchedExec(vm, sched, task) == STaskStatusYield);
  assert(task->ar->pc == start_pc+3); // test failed

  // 5 < 6 == true
  assert(task->ar->registry[0].value.n == 5.0);
  task->ar->registry[1].value.n = 6.0;
  task->ar->pc = start_pc;
  instructions[instr_offs]   = SInstr_LT(0, 0, 1); // 5 < 6
  instructions[instr_offs+1] = SInstr_JUMP(1);
  instructions[instr_offs+2] = SInstr_YIELD(0, 0, 0); // if test failed
  instructions[instr_offs+3] = SInstr_YIELD(0, 0, 0); // if test succeeded
  assert(SchedExec(vm, sched, task) == STaskStatusYield);
  assert(task->ar->pc == start_pc+4); // test succeeded

  // 5 <= 4 == false
  assert(task->ar->registry[0].value.n == 5.0);
  task->ar->registry[1].value.n = 4.0;
  task->ar->pc = start_pc;
  instructions[instr_offs]   = SInstr_LE(0, 0, 1); // 5 < 4
  instructions[instr_offs+1] = SInstr_JUMP(1);
  instructions[instr_offs+2] = SInstr_YIELD(0, 0, 0); // if test failed
  instructions[instr_offs+3] = SInstr_YIELD(0, 0, 0); // if test succeeded
  assert(SchedExec(vm, sched, task) == STaskStatusYield);
  assert(task->ar->pc == start_pc+3); // test failed

  // 5 <= 5 == true
  assert(task->ar->registry[0].value.n == 5.0);
  task->ar->registry[1].value.n = 5.0;
  task->ar->pc = start_pc;
  instructions[instr_offs]   = SInstr_LE(0, 0, 1); // 5 <= 5
  instructions[instr_offs+1] = SInstr_JUMP(1);
  instructions[instr_offs+2] = SInstr_YIELD(0, 0, 0); // if test failed
  instructions[instr_offs+3] = SInstr_YIELD(0, 0, 0); // if test succeeded
  assert(SchedExec(vm, sched, task) == STaskStatusYield);
  assert(task->ar->pc == start_pc+4); // test succeeded

  // 5 <= 6 == true
  assert(task->ar->registry[0].value.n == 5.0);
  task->ar->registry[1].value.n = 6.0;
  task->ar->pc = start_pc;
  instructions[instr_offs]   = SInstr_LE(0, 0, 1); // 5 <= 6
  instructions[instr_offs+1] = SInstr_JUMP(1);
  instructions[instr_offs+2] = SInstr_YIELD(0, 0, 0); // if test failed
  instructions[instr_offs+3] = SInstr_YIELD(0, 0, 0); // if test succeeded
  assert(SchedExec(vm, sched, task) == STaskStatusYield);
  assert(task->ar->pc == start_pc+4); // test succeeded

  SSchedDestroy(sched);
  STaskRelease(task);
  SFuncDestroy(func);
}


void test_control_flow(SVM* vm) {
  // Covered: YIELD(0), RETURN
  SValue constants[] = {
    SValueNumber(5),
  };
  SInstr instructions[100] = {
    SInstr_YIELD(0, 0, 0),
    SInstr_RETURN(0, 0),
  };

  SFunc* func = SFuncCreate(constants, instructions);
  SSched* sched = SSchedCreate();
  STask* task = STaskCreate(func, 0, 0);
  
  // YIELD resources
  assert(SchedExec(vm, sched, task) == STaskStatusYield);

  // AR's PC should be 1 less than the PC when this callback function is called.
  assert(task->ar->pc == instructions);

  // AR's PC should be at the first YIELD instruction
  assert(SInstrGetOP(*task->ar->pc) == S_OP_YIELD);
  assert(SInstrGetA(*task->ar->pc) == 0); // yield resources

  // RETURN
  assert(SchedExec(vm, sched, task) == STaskStatusEnd);

  SSchedDestroy(sched);
  STaskRelease(task);
  SFuncDestroy(func);
}

int main(int argc, const char** argv) {
  SVM vm = SVM_INIT;

  test_data(&vm);
  test_arithmetic(&vm);
  test_logic_tests(&vm);
  test_control_flow(&vm);

  return 0;
}
