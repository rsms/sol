#include "tidmap.h"

#define HASMAP_SIZE_T STask2ID
#include "hashmap.h"

typedef struct _Entry {
  STask2ID tid;
} _Entry;

// The following is based of off what DEFINE_HASHMAP(_Map, _Entry*) generates:
//extern const uint32_t __MapPrimes[];
typedef struct {
  uint32_t size;
  uint8_t nth_prime;
  _Entry* *entries;
} _MapBucket;

typedef struct {
  uint32_t size;
  uint8_t nth_prime;
  _MapBucket *entries;
} _Map;

void _MapNew(_Map *map);
void _MapDestroy(_Map *map);
bool _MapEnsureSize(_Map *map, uint32_t capacity);
bool _MapFind(const _Map *map, _Entry* **entry);
HashMapPutResult _MapPut(_Map *map, _Entry* **entry,
                         HashMapDuplicateResolution dr);
bool _MapRemove(_Map *map, _Entry* *entry);
// end of DEFINE_HASHMAP
//DEFINE_HASHMAP(_Map, _Entry*)

#define TID_CMP(left, right) ((*left)->tid == (*right)->tid ? 0 : 1)
#define TID_HASH(entry) (*entry)->tid

typedef _Entry* _HashType_Map;
DECLARE_HASHMAP(_Map, TID_CMP, TID_HASH, free, realloc)

// Then, "prefix" each key with the scheduler id, so that we can avoid shared
// maps across os threads.

void STIDMapFree(STIDMap* m) {
  _MapDestroy((_Map*)m);
}

// Returns false if task is already in the map
bool STIDMapAdd(STIDMap* m, STask2* t) {
  //printf("..put %u at %p\n", t->tid, t);
  _Entry** p = (_Entry**)&t;
  #if S_DEBUG
  if (_MapPut((_Map*)m, &p, HMDR_FAIL) == HMPR_FAILED) {
    assert((STask2*)*p == t); // or we have two live tasks with the same TID
    return false;
  } else {
    return true;
  }
  #else /* When compiling w/o assertions, allow a tail call */
  return _MapPut((_Map*)m, &p, HMDR_FAIL) != HMPR_FAILED;
  #endif
}

STask2* STIDMapGet(const STIDMap* m, STask2ID tid) {
  _Entry* p = (_Entry*)&tid;
  _Entry** pp = &p;
  if (_MapFind((const _Map*)m, &pp)) {
    //printf("..found %u at %p\n", tid, *pp);
    return (STask2*)*pp;
  } else {
    return 0;
  }
}

STask2* STIDMapRemove(STIDMap* m, STask2ID tid){
  _Entry* p = (_Entry*)&tid;
  if (_MapRemove((_Map*)m, &p)) {
    //printf("..removed %u at %p\n", tid, p);
    return (STask2*)p;
  } else {
    return 0;
  }
}
