#ifndef S_INSTR_H_
#define S_INSTR_H_
#include <sol/common.h>

// We currently only support LE archs with 32-bit or larger registers
#if S_TARGET_ARCH_SIZE < 32
  #error "Unsupported target architecture: Register size too small"
#elif S_TARGET_ENDIAN != S_ENDIAN_LITTE
  #error "Unsupported target architecture: Big Endian"
#endif

//
// One instruction is 32 bits:
typedef uint32_t SInstr;
//
// Each instruction is encoded in one of three ways:
//
//                              OP A B C
//              Operation OP with operands A, B and C
//
// | 0        5 | 6          13 | 14           22 | 23           31 |
// |------------|---------------|-----------------|-----------------|
// |     OP     |       A       |        B        |        C        |
// |------------|---------------|-----------------------------------|
//       6              8                9                 9
//    [0..63]        [0..255]         [0..511]          [0..511]
//
//                              OP A Bx
//              Operation OP with operands A and Bs|Bu
//
// | 0        5 | 6          13 | 14                             31 |
// |------------|---------------|-----------------------------------|
// |     OP     |       A       |              Bs/Bu                |
// |------------|---------------|-----------------------------------|
//       6              8                        18
//    [0..63]        [0..255]             Bu: [0..262143]
//                                        Bu: [-131071..131072]
//
//                               OP Bxx
//                 Operation OP with operand Bss|Buu
//
// | 0        5 | 6                                              31 |
// |------------|---------------------------------------------------|
// |     OP     |                     Bss/Buu                       |
// |------------|---------------------------------------------------|
//       6                               26
//    [0..63]                   Buu: [0..67108863]
//                              Bss: [-33554431..33554432]
//
// There is room for 64 operations and 256 registers (OP=6 bits, A=8 bits)
//
#define S_INSTR_DEFINE(_) \
  _(RETURN,     AB_) /* return R(A), ... ,R(A+B-2) */\
  _(YIELD,      AB_) /* suspend and reschedule */\
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

// Each instruction will have a corresponding operation code identified by an
// enum value "S_OP_<name>"
typedef enum {
  #define I_ENUM(name, args) S_OP_##name,
  S_INSTR_DEFINE(I_ENUM)
  #undef I_ENUM
} S_OP_V_;

// Macros to compose instructions
#define S_INSTR_A__(OP, A) \
  (((SInstr)(OP)) | \
   (((SInstr)(A) << S_INSTR_OP_SIZE) & S_INSTR_A_MASK) \
  )

#define S_INSTR_AB_(OP, A, B) \
  (((SInstr)(OP)) | \
   (((SInstr)(A) << S_INSTR_OP_SIZE) & S_INSTR_A_MASK) | \
   (((SInstr)(B) << (S_INSTR_OP_SIZE + S_INSTR_A_SIZE)) & S_INSTR_B_MASK) \
  )

#define S_INSTR_ABC(OP, A, B, C) \
  (((SInstr)(OP)) | \
   (((SInstr)(A) << S_INSTR_OP_SIZE) & S_INSTR_A_MASK) | \
   (((SInstr)(B) << (S_INSTR_OP_SIZE + S_INSTR_A_SIZE)) & S_INSTR_B_MASK) | \
   ((SInstr)(C) << (S_INSTR_OP_SIZE + S_INSTR_A_SIZE + S_INSTR_B_SIZE)) \
  )

#define S_INSTR_ABu(OP, A, Bu) \
  (((SInstr)(OP)) | \
   (((SInstr)(A) << S_INSTR_OP_SIZE) & S_INSTR_A_MASK) | \
   ((SInstr)(Bu) << (S_INSTR_OP_SIZE + S_INSTR_A_SIZE)) \
  )

#define S_INSTR_ABs(OP, A, Bs) \
  S_INSTR_ABu((OP), (A), ((uint32_t)(Bs) + (S_INSTR_Bu_MAX / 2)) )

#define S_INSTR_Buu(OP, Buu) \
  (((SInstr)(OP)) | \
   ((SInstr)(Buu) << S_INSTR_OP_SIZE) \
  )

#define S_INSTR_Bss(OP, Bss) \
  S_INSTR_Buu((OP), ((uint32_t)(Bss) + (S_INSTR_Buu_MAX / 2)) )

// Macros for accessing instruction field values
#define SInstrGetOP(i)  ((uint8_t)((i) & S_INSTR_OP_MASK))
#define SInstrGetA(i)   ((uint8_t)(((i) & S_INSTR_A_MASK)  >> S_INSTR_A_OFFS))
#define SInstrGetB(i)   ((uint16_t)(((i) & S_INSTR_B_MASK)  >> S_INSTR_B_OFFS))
#define SInstrGetC(i)   ((uint16_t)(((i) & S_INSTR_C_MASK)  >> S_INSTR_C_OFFS))
#define SInstrGetBu(i)  ((uint32_t)(((i) & S_INSTR_Bu_MASK) >> S_INSTR_Bu_OFFS))
#define SInstrGetBuu(i) ((uint32_t)((i) >> S_INSTR_OP_SIZE))
#define SInstrGetBs(i)  ((int32_t)(SInstrGetBu(i) - (S_INSTR_Bu_MAX/2)))
#define SInstrGetBss(i) ((int32_t)(SInstrGetBuu(i) - (S_INSTR_Buu_MAX/2)))

// Instruction component constants
// Operation code
#define S_INSTR_OP_SIZE 6
#define S_INSTR_OP_MASK 0x3f       // 000000000 000000000 00000000 111111
#define S_INSTR_OP_MAX  0x3f
// Field A
#define S_INSTR_A_SIZE  8
#define S_INSTR_A_OFFS  6
#define S_INSTR_A_MASK  0x3fc0     // 000000000 000000000 11111111 000000
#define S_INSTR_A_MAX   0xff
// Field B
#define S_INSTR_B_SIZE  9
#define S_INSTR_B_OFFS  14
#define S_INSTR_B_MASK  0x7fc000   // 000000000 111111111 00000000 000000
#define S_INSTR_B_MAX   0x1ff
// Field C
#define S_INSTR_C_SIZE  9
#define S_INSTR_C_OFFS  23
#define S_INSTR_C_MASK  0xff800000 // 111111111 000000000 00000000 000000
#define S_INSTR_C_MAX   0x1ff
// Field Bu
#define S_INSTR_Bu_SIZE 18
#define S_INSTR_Bu_MASK 0xffffc000 // 111111111111111111 00000000 000000
#define S_INSTR_Bu_OFFS 14
#define S_INSTR_Bu_MAX  0x3ffff
// Field Bs
#define S_INSTR_Bs_SIZE S_INSTR_Bu_SIZE
#define S_INSTR_Bs_MASK S_INSTR_Bu_MASK
#define S_INSTR_Bs_OFFS S_INSTR_Bu_OFFS
#define S_INSTR_Bs_MIN  (-(S_INSTR_Bu_MAX/2))
#define S_INSTR_Bs_MAX  ((-S_INSTR_Bs_MIN)+1)
// Field Buu
#define S_INSTR_Buu_SIZE 26
#define S_INSTR_Buu_MASK 0xffffffc0 // 11111111111111111111111111 000000
#define S_INSTR_Buu_OFFS 6
#define S_INSTR_Buu_MAX  0x3ffffff
// Field Bss
#define S_INSTR_Bss_SIZE S_INSTR_Buu_SIZE
#define S_INSTR_Bss_MASK S_INSTR_Buu_MASK
#define S_INSTR_Bss_OFFS S_INSTR_Buu_OFFS
#define S_INSTR_Bss_MIN  (-(S_INSTR_Buu_MAX/2))
#define S_INSTR_Bss_MAX  ((-S_INSTR_Bss_MIN)+1)

// Memory layout
// ABC:
//
//  31     23 22     14 13     6 5    0
//  000000000 000000000 00000000 000000   C=0, B=0, A=0, OP=0
//  MSB                             LSB
//
// ABu:
//
//   31              14 13     6 5    0
//   000000000000000000 00000000 000000   Bu=0, A=0, OP=0
//   MSB                            LSB
//
// Buu:
//
//    31                       6 5    0
//    00000000000000000000000000 000000   Buu=0, OP=0
//    MSB                           LSB
//

// Define convenience functions for each instruction. These functions are
// constant expressions, so they can be used as constant initializers. However,
// they are not compile-time constants.
#define FHEAD inline static SInstr S_UNUSED S_ALWAYS_INLINE S_PURE S_WUNUSEDR
#define APPLY_ABC(name) \
  FHEAD SInstr_##name(uint8_t A, uint16_t B, uint16_t C) { \
    return S_INSTR_ABC(S_OP_##name, A, B, C); \
  }
#define APPLY_AB_(name) \
  FHEAD SInstr_##name(uint8_t A, uint16_t B) { \
    return S_INSTR_AB_(S_OP_##name, A, B); \
  }
#define APPLY_A__(name) \
  FHEAD SInstr_##name(uint8_t A) { \
    return S_INSTR_A__(S_OP_##name, A); \
  }
#define APPLY_ABu(name) \
 FHEAD SInstr_##name(uint8_t A, uint32_t Bu) { \
   return S_INSTR_ABu(S_OP_##name, A, Bu); \
 }
#define APPLY_ABs(name) \
  FHEAD SInstr_##name(uint8_t A, int32_t Bs) { \
    return S_INSTR_ABs(S_OP_##name, A, Bs); \
  }
#define APPLY_Buu(name) \
  FHEAD SInstr_##name(uint32_t Buu) { \
    return S_INSTR_Buu(S_OP_##name, Buu); \
  }
#define APPLY_Bss(name) \
  FHEAD SInstr_##name(uint32_t Bss) { \
    return S_INSTR_Bss(S_OP_##name, Bss); \
  }
#define APPLY(name, args) APPLY_##args(name)
S_INSTR_DEFINE(APPLY)
#undef APPLY
#undef APPLY_ABC
#undef APPLY_AB_
#undef APPLY_A__
#undef APPLY_ABu
#undef APPLY_ABs
#undef APPLY_Buu
#undef APPLY_Bss
#undef FHEAD


#endif // S_INSTR_H_
