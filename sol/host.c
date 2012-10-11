#include "host.h"

#if S_TARGET_OS_WINDOWS
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#elif S_TARGET_OS_POSIX
  #include <unistd.h> // sysconf
  #include <sys/types.h>
  #include <sys/sysctl.h>
#endif

uint32_t SHostAvailCPUCount() {
  // Thanks to http://stackoverflow.com/questions/150355/programmatically-
  //           find-the-number-of-cores-on-a-machine

  #if S_TARGET_OS_WINDOWS
    // TODO: dry code needs testing
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return (sysinfo.dwNumberOfProcessors < 1) ?
      0 : sysinfo.dwNumberOfProcessors;
  
  #elif S_TARGET_OS_POSIX && defined(HW_NCPU)
    uint32_t ncpu = 0;
    size_t ncpusz = sizeof(ncpu);
    int key[4];
    key[0] = CTL_HW;

    #ifdef HW_AVAILCPU
    key[1] = HW_AVAILCPU;
    if (sysctl(key, 2, &ncpu, &ncpusz, NULL, 0) == -1) {
      // Failed. Try HW_NCPU
    #endif
      key[1] = HW_NCPU;
      if (sysctl(key, 2, &ncpu, &ncpusz, NULL, 0 ) == -1) {
        return 0;
      }
    #ifdef HW_AVAILCPU
    }
    #endif
    return ncpu;

  #elif defined(_SC_NPROCESSORS_ONLN)
    // Possibly available on OS X, Linux, Solaris and AIX
    long ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    if (ncpu < 1) {
    #ifdef _SC_NPROCESSORS_CONF
      ncpu = sysconf(_SC_NPROCESSORS_CONF);
      if (ncpu < 1) {
        ncpu = 0;
      }
    #else
      ncpu = 0;
    #endif
    }
    return (uint32_t)ncpu;

  #else
    #warning "Unsupported host"
    return 0;
  #endif
}
