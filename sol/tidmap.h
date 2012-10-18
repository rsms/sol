// Task ID map
#ifndef S_TIDMAP_H_
#define S_TIDMAP_H_
#include <sol/task.h>

typedef struct {
  STaskID  size;
  uint8_t  opaque0;
  void*    opaque1;
} STIDMap;

#define S_TID_MAP_INIT (STIDMap){0,0,0}

// Free any _internal_ data, but not the map itself.
void STIDMapFree(STIDMap* m);

// Returns false if task is already in the map.
bool STIDMapAdd(STIDMap* m, STask* t);

// Lookup task for `tid`. Returns 0 if not found.
STask* STIDMapGet(const STIDMap* m, STaskID tid);

// Lookup and remove any task mapped to `tid`. Returns the task that was removed
// or 0 if not found.
STask* STIDMapRemove(STIDMap* m, STaskID tid);


#endif // S_TIDMAP_H_
