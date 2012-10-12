// Virtual Machine
#ifndef S_VM_H_
#define S_VM_H_
#include <sol/common.h>
//#include <sol/value.h>

// typedef struct {
//   SValue values[100];
//   size_t size;
//   size_t count;
// } SConstants;

typedef struct {
  // TODO: own schedulers
} SVM;

#define SVM_INIT ((SVM){ \
  /*.constants = { .values = {}, .size = 100, .count = 0 }*/ \
})

#endif // S_VM_H_
