// Task ID map
#ifndef S_TIDMAP_H_
#define S_TIDMAP_H_

#include <sol/common.h>
//#include <sol/task.h>

#define STask2IDMax UINT32_MAX
typedef uint32_t STask2ID;

typedef struct STask2 {
  STask2ID tid;
} STask2;

typedef struct {
  STask2ID  size;
  uint8_t   opaque0;
  void*     opaque1;
} STIDMap;

#define S_TID_MAP_INIT (STIDMap){0,0,0}

// Free any _internal_ data, but not the map itself.
void STIDMapFree(STIDMap* m);

// Returns false if task is already in the map.
bool STIDMapAdd(STIDMap* m, STask2* t);

// Lookup task for `tid`. Returns 0 if not found.
STask2* STIDMapGet(const STIDMap* m, STask2ID tid);

// Lookup and remove any task mapped to `tid`. Returns the task that was removed
// or 0 if not found.
STask2* STIDMapRemove(STIDMap* m, STask2ID tid);


#endif // S_TIDMAP_H_
