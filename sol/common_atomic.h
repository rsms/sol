#ifndef S_COMMON_ATOMIC_H_
#define S_COMMON_ATOMIC_H_
#ifndef S_INTERNAL_
#error "This file should not be included directly"
#endif

// Atomically swap integers or pointers in memory.
// E.g: int old_value = SAtomicSwap(&value, new_value);
// T SAtomicSwap(T *ptr, T value)
#if S_WITHOUT_SMP
  #define SAtomicSwap(ptr, value)  \
    ({ __typeof__ (value) oldval = *(ptr); \
       *(ptr) = (value); \
       oldval; })
#elif defined(__clang__)
  // This is more efficient than the below fallback
  #define SAtomicSwap __sync_swap
#elif defined(__GNUC__) && (__GNUC__ >= 4)
  static inline void* S_UNUSED _SAtomicSwap(void* volatile* ptr, void* value) {
    void* oldval;
    do {
      oldval = *ptr;
    } while (__sync_val_compare_and_swap(ptr, oldval, value) != oldval);
    return oldval;
  }
  #define SAtomicSwap(ptr, value) \
    _SAtomicSwap((void* volatile*)(ptr), (void*)(value))
#else
  #error "Unsupported compiler: Missing support for atomic operations"
#endif

// Atomically increment a 32-bit integer by N. There's no return value.
// void SAtomicSubAndFetch(T* operand, T delta)
#if S_WITHOUT_SMP
  #define SAtomicAdd32(operand, delta) (*(operand) += (delta))
#elif S_TARGET_ARCH_X64 || S_TARGET_ARCH_X86
  inline static void S_UNUSED SAtomicAdd32(int32_t* operand, int32_t delta) {
    // From http://www.memoryhole.net/kyle/2007/05/atomic_incrementing.html
    __asm__ __volatile__ (
      "lock xaddl %1, %0\n" // add delta to operand
      : // no output
      : "m" (*operand), "r" (delta)
    );
  }
#elif defined(__clang__) || (defined(__GNUC__) && (__GNUC__ >= 4))
  #define SAtomicAdd32 __sync_sub_and_fetch
#else
  #error "Unsupported compiler: Missing support for atomic operations"
#endif

// Subtract `delta` from `operand` and return the resulting value of `operand`
// T SAtomicSubAndFetch(T* operand, T delta)
#if S_WITHOUT_SMP
  #define SAtomicSubAndFetch(operand, delta) (*(operand) -= (delta))
#elif defined(__clang__) || (defined(__GNUC__) && (__GNUC__ >= 4))
  #define SAtomicSubAndFetch __sync_sub_and_fetch
#else
  #error "Unsupported compiler: Missing support for atomic operations"
#endif

#endif // S_COMMON_ATOMIC_H_
