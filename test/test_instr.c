#include <sol/common.h>
#include <sol/sched.h>
#include <sol/debug.h>

int main(int argc, const char** argv) {
  SInstr ins;

  // Test ABC instruction values
  ins = S_INSTR_ABC(S_OP_JUMP, S_INSTR_A_MAX, S_INSTR_B_MAX, S_INSTR_C_MAX);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetA(ins) == S_INSTR_A_MAX);
  assert(SInstrGetB(ins) == S_INSTR_B_MAX);
  assert(SInstrGetC(ins) == S_INSTR_C_MAX);

  // Test value overflow
  ins = S_INSTR_ABC(S_OP_JUMP, S_INSTR_A_MAX+2, S_INSTR_B_MAX, S_INSTR_C_MAX);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetA(ins) != S_INSTR_A_MAX);
  assert(SInstrGetA(ins) != S_INSTR_A_MAX+2);
  assert(SInstrGetB(ins) == S_INSTR_B_MAX);
  assert(SInstrGetC(ins) == S_INSTR_C_MAX);

  ins = S_INSTR_ABC(S_OP_JUMP, S_INSTR_A_MAX, S_INSTR_B_MAX+2, S_INSTR_C_MAX);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetA(ins) == S_INSTR_A_MAX);
  assert(SInstrGetB(ins) != S_INSTR_B_MAX);
  assert(SInstrGetB(ins) != S_INSTR_B_MAX+2);
  assert(SInstrGetC(ins) == S_INSTR_C_MAX);

  ins = S_INSTR_ABC(S_OP_JUMP, S_INSTR_A_MAX, S_INSTR_B_MAX, S_INSTR_C_MAX+2);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetA(ins) == S_INSTR_A_MAX);
  assert(SInstrGetB(ins) == S_INSTR_B_MAX);
  assert(SInstrGetC(ins) != S_INSTR_C_MAX);
  assert(SInstrGetC(ins) != S_INSTR_C_MAX+2);


  // Test AB instruction values
  ins = S_INSTR_AB(S_OP_JUMP, S_INSTR_A_MAX, S_INSTR_B_MAX);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetA(ins) == S_INSTR_A_MAX);
  assert(SInstrGetB(ins) == S_INSTR_B_MAX);
  assert(SInstrGetC(ins) == 0);

  ins = S_INSTR_AB(S_OP_JUMP, S_INSTR_A_MAX+2, S_INSTR_B_MAX);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetA(ins) != S_INSTR_A_MAX);
  assert(SInstrGetA(ins) != S_INSTR_A_MAX+2);
  assert(SInstrGetB(ins) == S_INSTR_B_MAX);
  assert(SInstrGetC(ins) == 0);

  ins = S_INSTR_AB(S_OP_JUMP, S_INSTR_A_MAX, S_INSTR_B_MAX+2);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetA(ins) == S_INSTR_A_MAX);
  assert(SInstrGetB(ins) != S_INSTR_B_MAX);
  assert(SInstrGetB(ins) != S_INSTR_B_MAX+2);
  assert(SInstrGetC(ins) == 0);


  // Test A instruction value
  ins = S_INSTR_A(S_OP_JUMP, S_INSTR_A_MAX);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetA(ins) == S_INSTR_A_MAX);
  assert(SInstrGetB(ins) == 0);
  assert(SInstrGetC(ins) == 0);

  ins = S_INSTR_A(S_OP_JUMP, S_INSTR_A_MAX+2);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetA(ins) != S_INSTR_A_MAX);
  assert(SInstrGetA(ins) != S_INSTR_A_MAX+2);
  assert(SInstrGetB(ins) == 0);
  assert(SInstrGetC(ins) == 0);


  // Test ABu instruction value
  ins = S_INSTR_ABu(S_OP_JUMP, S_INSTR_A_MAX, S_INSTR_Bu_MAX);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetA(ins) == S_INSTR_A_MAX);
  assert(SInstrGetBu(ins) == S_INSTR_Bu_MAX);

  ins = S_INSTR_ABu(S_OP_JUMP, S_INSTR_A_MAX+2, S_INSTR_Bu_MAX);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetA(ins) != S_INSTR_A_MAX);
  assert(SInstrGetA(ins) != S_INSTR_A_MAX+2);
  assert(SInstrGetBu(ins) == S_INSTR_Bu_MAX);

  ins = S_INSTR_ABu(S_OP_JUMP, S_INSTR_A_MAX, S_INSTR_Bu_MAX+2);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetA(ins) == S_INSTR_A_MAX);
  assert(SInstrGetBu(ins) != S_INSTR_Bu_MAX);
  assert(SInstrGetBu(ins) != S_INSTR_Bu_MAX+2);


  // Test ABs instruction value
  ins = S_INSTR_ABs(S_OP_JUMP, S_INSTR_A_MAX, S_INSTR_Bs_MIN);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetA(ins) == S_INSTR_A_MAX);
  assert(SInstrGetBs(ins) == S_INSTR_Bs_MIN);

  ins = S_INSTR_ABs(S_OP_JUMP, S_INSTR_A_MAX, S_INSTR_Bs_MAX);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetA(ins) == S_INSTR_A_MAX);
  assert(SInstrGetBs(ins) == S_INSTR_Bs_MAX);

  ins = S_INSTR_ABs(S_OP_JUMP, S_INSTR_A_MAX+2, S_INSTR_Bs_MIN);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetA(ins) != S_INSTR_A_MAX);
  assert(SInstrGetA(ins) != S_INSTR_A_MAX+2);
  assert(SInstrGetBs(ins) == S_INSTR_Bs_MIN);

  ins = S_INSTR_ABs(S_OP_JUMP, S_INSTR_A_MAX, S_INSTR_Bs_MIN-2);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetA(ins) == S_INSTR_A_MAX);
  assert(SInstrGetBs(ins) != S_INSTR_Bs_MIN);
  assert(SInstrGetBs(ins) != S_INSTR_Bs_MIN-2);


  // Test Buu instruction value
  ins = S_INSTR_Buu(S_OP_JUMP, S_INSTR_Buu_MAX);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetBuu(ins) == S_INSTR_Buu_MAX);

  ins = S_INSTR_Buu(S_OP_JUMP, S_INSTR_Buu_MAX+2);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetBuu(ins) != S_INSTR_Buu_MAX);
  assert(SInstrGetBuu(ins) != S_INSTR_Buu_MAX+2);


  // Test Bss instruction value
  ins = S_INSTR_Bss(S_OP_JUMP, S_INSTR_Bss_MIN);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetBss(ins) == S_INSTR_Bss_MIN);

  ins = S_INSTR_Bss(S_OP_JUMP, S_INSTR_Bss_MIN-2);
  assert(SInstrGetOP(ins) == S_OP_JUMP);
  assert(SInstrGetBss(ins) != S_INSTR_Bss_MIN);
  assert(SInstrGetBss(ins) != S_INSTR_Bss_MIN-2);

  return 0;
}
