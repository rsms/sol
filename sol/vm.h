#ifndef S_VM_H_
#define S_VM_H_
#include <sol/common.h>
#include <sol/task.h>

// Resumes execution of `program`.
STaskStatus SVMExec(STask *t);

#endif // S_VM_H_
