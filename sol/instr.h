#ifndef S_INSTR_H_
#define S_INSTR_H_
#include <sol/common.h>

//
// One instruction is 32 bits and are encoded in one of three ways:
//
//                              OP A B C
//             Operation OP with operands A, B and C
//
// | 0        5 | 6          13 | 14           22 | 23           31 |
// |------------|---------------|-----------------|-----------------|
// |     OP     |       A       |        B        |        C        |
// |------------|---------------|-----------------------------------|
//       6              8                9                 9
//
//
//                             OP A Bs/Bu
//   Operation OP with operand A with signed/unsigned integer Bs/Bu
//
// | 0        5 | 6          13 | 14                             31 |
// |------------|---------------|-----------------------------------|
// |     OP     |       A       |              Bs/Bu                |
// |------------|---------------|-----------------------------------|
//       6              8                        18
//
//
//                             OP Bss/Buu
//          Operation OP with signed/unsigned integer Bss/Buu
//
// | 0        5 | 6                                              31 |
// |------------|---------------------------------------------------|
// |     OP     |                     Bss/Buu                       |
// |------------|---------------------------------------------------|
//       6                               26
//
// There is room for 64 operations and 256 registers (OP=6 bits, A=8 bits)
//

// An instruction
typedef uint32_t SInstr;

// Instruction definitions
#define S_INSTR_DEFINE(_) \
  _(RETURN,     AB_) /* return R(A), ... ,R(A+B-2) */\
  _(YIELD,      AB_) /* results R(A), ... ,R(B) */\
  _(MOVE,       AB_) /* R(A) = R(B) */\
  _(LOADK,      ABu) /* R(A) = K(Bu) */\
  _(ADDI,       ABC) /* R(A) = RK(B) + RK(C) */\
  _(SUBI,       ABC) /* R(A) = RK(B) - RK(C) */\
  _(MULI,       ABC) /* R(A) = RK(B) * RK(C) */\
  _(DIVI,       ABC) /* R(A) = RK(B) / RK(C) */\
  _(NOT,        AB_) /* R(A) = not R(B) */\
  _(EQ,         ABC) /* if ((RK(B) == RK(C)) ~= A) then PC++ */\
  _(LT,         ABC) /* if ((RK(B) < RK(C)) ~= A) then PC++ */\
  _(LE,         ABC) /* if ((RK(B) <= RK(C)) ~= A) then PC++ */\
  _(JUMP,       Bss) /* PC += Bss */

// Each instruction will cause the following to be defined:
//
// - A unique operation code [0-63] under the name SInstrOP_<name>
//
// - A function "SInstr SInstr_<name>(...)" The arguments the function accepts
//   will match the OP's arguments. For instance, an OP defined with arguments
//   "AB_" will have a "SInstr SInstr_<name>(uint8_t A, uint16_t B)" function.
//

// The following macro functions are available for reading individual values
// of instructions:
//
//   uint8_t   SInstrGetOP  (SInstr i)
//   uint8_t   SInstrGetA   (SInstr i)
//   uint16_t  SInstrGetB   (SInstr i)
//   uint16_t  SInstrGetC   (SInstr i)
//   int32_t   SInstrGetBs  (SInstr i)
//   uint32_t  SInstrGetBu  (SInstr i)
//   int32_t   SInstrGetBss (SInstr i)
//   uint32_t  SInstrGetBuu (SInstr i)
//

// Operation values
typedef enum {
  #define I_ENUM(name, args) SInstrOP_##name,
  S_INSTR_DEFINE(I_ENUM)
  #undef I_ENUM
} S_OP_V_;

//
//  31     23 22     14 13     6 5    0
//  000000000 000000000 00000000 000000   C=0, B=0, A=0, OP=0
//  000000000 000000000 00000000 000011   C=0, B=0, A=0, OP=3
//  000000000 000000000 00000010 000011   C=0, B=0, A=2, OP=3
//
#define S_INSTR_OP_SIZE 6
#define S_INSTR_OP_MASK 0x3f       // 000000000 000000000 00000000 111111
#define S_INSTR_OP_MAX  0x3f

#define S_INSTR_A_SIZE  8
#define S_INSTR_A_OFFS  6
#define S_INSTR_A_MASK  0x3fc0     // 000000000 000000000 11111111 000000
#define S_INSTR_A_MAX   0xff

#define S_INSTR_B_SIZE  9
#define S_INSTR_B_OFFS  14
#define S_INSTR_B_MASK  0x7fc000   // 000000000 111111111 00000000 000000
#define S_INSTR_B_MAX   0x1ff

#define S_INSTR_C_SIZE  9
#define S_INSTR_C_OFFS  23
#define S_INSTR_C_MASK  0xff800000 // 111111111 000000000 00000000 000000
#define S_INSTR_C_MAX   0x1ff
//
//  31              14 13     6 5    0
//  000000000000000000 00000000 000000   Bx=0, A=0, OP=0
//
#define S_INSTR_Bx_SIZE 18
#define S_INSTR_Bx_MASK 0xffffc000 // 111111111111111111 00000000 000000
#define S_INSTR_Bx_OFFS 14
#define S_INSTR_Bx_MAX  0x3ffff
//
//  31                       6 5    0
//  00000000000000000000000000 000000   Bxx=0, OP=0
//
#define S_INSTR_Bxx_SIZE 26
#define S_INSTR_Bxx_MASK 0xffffffc0 // 11111111111111111111111111 000000
#define S_INSTR_Bxx_OFFS 6
#define S_INSTR_Bxx_MAX  0x3ffffff

// Functions for encoding values
#define A__(name)\
  static inline SInstr SInstr_##name(uint8_t A) { \
    SInstr oc = 0; \
    oc |= A; oc <<= S_INSTR_OP_SIZE; \
    oc |= SInstrOP_##name; \
    return oc; \
  }

#define AB_(name) \
  static inline SInstr SInstr_##name(uint8_t A, uint16_t B) { \
    SInstr oc = 0; \
    oc |= B; oc <<= S_INSTR_A_SIZE; \
    oc |= A; oc <<= S_INSTR_OP_SIZE; \
    oc |= SInstrOP_##name; \
    return oc; \
  }

#define ABC(name) \
  static inline SInstr SInstr_##name(uint8_t A, uint16_t B, uint16_t C) { \
    SInstr oc = 0; \
    oc |= C; oc <<= S_INSTR_B_SIZE; \
    oc |= B; oc <<= S_INSTR_A_SIZE; \
    oc |= A; oc <<= S_INSTR_OP_SIZE; \
    oc |= SInstrOP_##name; \
    return oc; \
  }

#define ABs(name) \
  static inline SInstr SInstr_##name(uint8_t A, int32_t Bs) { \
    SInstr oc = 0; \
    oc |= (uint32_t)Bs; oc <<= S_INSTR_A_SIZE;\
    oc |= A; oc <<= S_INSTR_OP_SIZE; \
    oc |= SInstrOP_##name; \
    return oc; \
  }

#define ABu(name) \
  static inline SInstr SInstr_##name(uint8_t A, uint32_t Bu) { \
    SInstr oc = 0; \
    oc |= Bu; oc <<= S_INSTR_A_SIZE;\
    oc |= A; oc <<= S_INSTR_OP_SIZE; \
    oc |= SInstrOP_##name; \
    return oc; \
  }

#define Bss(name) \
  static inline SInstr SInstr_##name(int32_t Bss) { \
    SInstr oc = 0; \
    oc |= Bss; oc <<= S_INSTR_OP_SIZE; \
    oc |= SInstrOP_##name; \
    return oc; \
  }

#define Buu(name) \
  static inline SInstr SInstr_##name(uint32_t Buu) { \
    SInstr oc = 0; \
    oc |= Buu; oc <<= S_INSTR_OP_SIZE; \
    oc |= SInstrOP_##name; \
    return oc; \
  }

#define DEF_ENC_FUN(name, operands) operands(name)
S_INSTR_DEFINE(DEF_ENC_FUN)
#undef DEF_ENC_FUN

#undef A__
#undef AB_
#undef ABC
#undef ABu
#undef ABs
#undef Bss
#undef Buu

// OpCode field accessors

#define SInstrGetOP(oc)  ( (oc) & S_INSTR_OP_MASK)

#define SInstrGetA(oc)   (((oc) & S_INSTR_A_MASK)  >> S_INSTR_A_OFFS)
#define SInstrGetB(oc)   (((oc) & S_INSTR_B_MASK)  >> S_INSTR_B_OFFS)
#define SInstrGetC(oc)   (((oc) & S_INSTR_C_MASK)  >> S_INSTR_C_OFFS)

#define SInstrGetBx(oc)  (((oc) & S_INSTR_Bx_MASK) >> S_INSTR_Bx_OFFS)
#define SInstrGetBxx(oc) ((oc) >> S_INSTR_OP_SIZE)

#define SInstrGetBs(oc)  (int32_t)SInstrGetBx(oc)
//#define SInstrGetBss(oc) (int32_t)SInstrGetBxx(oc)
#define SInstrGetBss(oc) (int32_t)((oc) & S_INSTR_Bxx_MASK)

#define SInstrGetBu(oc)  (uint32_t)SInstrGetBx(oc)
#define SInstrGetBuu(oc) (uint32_t)SInstrGetBxx(oc)

#endif // S_INSTR_H_
