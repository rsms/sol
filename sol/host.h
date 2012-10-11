#ifndef S_HOST_H_
#define S_HOST_H_
#include <sol/common.h>

// Number of available processing units (cores). Returns 0 on error.
// This value can change at runtime (e.g. from power management settings).
uint32_t SHostAvailCPUCount();

#endif
