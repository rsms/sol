/*
 * AUTHOR:  René Kijewski  (rene.<surname>@fu-berlin.de)
 * LICENSE: MIT
 */

#ifndef HASHMAP_H__
#define HASHMAP_H__

// Should be an unsigned integer
#ifndef HASMAP_SIZE_T
#define HASMAP_SIZE_T size_t
#endif

typedef enum {
    HMDR_FAIL = 0, // returns old entry in parameter entry, lets NAME##Put()
                   // "fail", i.e. return HMPR_FAILED
    HMDR_FIND,     // returns old entry in parameter entry
    HMDR_REPLACE,  // puts new entry, replaces current entry if exists
    HMDR_SWAP,     // puts new entry, swappes old entry with *entry otherwise
    HMDR_STACK,    // put an duplicate input the map (later you have to call
                   // delete multiple times)
} HashMapDuplicateResolution;

typedef enum {
    HMPR_FAILED = 0, // map could not grow
    HMPR_FOUND,      // item already existed
    HMPR_REPLACED,   // item was replace
    HMPR_SWAPPED,    // item already existed and was swapped with *entry
    HMPR_STACKED,    // new item was stacked in map, old value stored in *entry
    HMPR_PUT,        // new item was added to map
} HashMapPutResult;

typedef enum {
    _HMNPR_NOT_NEEDED,
    _HMNPR_FAIL,
    _HMNPR_GREW,
} _HashMapNextPrimeResult;

// http://oeis.org/A014234
// Buckets should mostly contain one element (if the hash function is good), so
// I put in 1 instead of 2.
#define _HASHMAP_PRIMES 1, 3, 7, 13, 31, 61, 127, 251, 509, 1021, 2039,        \
                        4093, 8191, 16381, 32749, 65521, 131071, 262139,       \
                        524287, 1048573, 2097143, 4194301, 8388593, 16777213,  \
                        33554393, 67108859, 134217689, 268435399, 536870909,   \
                        1073741789, 2147483647

#define _HashStructure(VALUE_TYPE)                                             \
struct {                                                                       \
    HASMAP_SIZE_T  size;                                                          \
    uint8_t     nth_prime;                                                     \
    VALUE_TYPE *entries;                                                       \
}

/**
 * Defines hashmap helper functions for type NAME.
 * \param NAME Typedef'd name of the HashMap type.
 * \param TYPE Type of the values to store.
 */
#define DEFINE_HASHMAP(NAME, TYPE)                                             \
                                                                               \
extern const HASMAP_SIZE_T _##NAME##Primes[];                                  \
                                                                               \
typedef TYPE _HashType##NAME;                                                  \
typedef _HashStructure(_HashType##NAME) NAME##Bucket;                          \
typedef _HashStructure(NAME##Bucket)    NAME;                                  \
                                                                               \
/* Initializes an empty hashmap.                                             */\
/* An null'ed map is initalized too, but has an empty capacity (which grows  */\
/* automatically.)                                                           */\
/* \param map [Out] Map to initialize                                        */\
void NAME##New(NAME *map);                                                     \
                                                                               \
/* Destroys a hash map, i.e.the map will have an size and capacity of 0      */\
/* after his call.                                                           */\
/* \param map Map to destroy.                                                */\
void NAME##Destroy(NAME *map);                                                 \
                                                                               \
/* Ensures the map can hold capacity much entries.                           */\
/* \param map Map to grow if needed.                                         */\
/* \param entry Entry to remove.                                             */\
/* \return false, if could not ensure size.                                  */\
bool NAME##EnsureSize(NAME *map, HASMAP_SIZE_T capacity);                      \
                                                                               \
/* Looks up an entry in a map.                                               */\
/* \param map Map to search in.                                              */\
/* \param entry [In/Out] Entry to search, returns pointer to found item      */\
/* \return false, if could not found.                                        */\
bool NAME##Find(const NAME *map,                                               \
                _HashType##NAME **entry);                                      \
                                                                               \
/* Adds an entry into a map.                                                 */\
/* \param map Map to add to.                                                 */\
/* \param entry [In/Out] Entry add. If duplicate, return pointer to it in    */\
/*              here.                                                        */\
/* \return false, if map could not grow                                      */\
HashMapPutResult NAME##Put(NAME *map,                                          \
                           _HashType##NAME **entry,                            \
                           HashMapDuplicateResolution dr);                     \
                                                                               \
/* Removes an entry for the list.                                            */\
/* \param map Map to remove from.                                            */\
/* \param entry [In/out] Entry to remove, returns removed entry.             */\
/* \return false, if did not exist                                           */\
bool NAME##Remove(NAME *map,                                                   \
                  _HashType##NAME *entry);

/**
 * To iterate over all entries in order they are saved in the map.
 * You must not insert or delete elements in this loop.
 * You can use continue and break as in usual for-loops.
 * 
 * You HAVE TO put braces:
 *     HASHMAP_FOR_EACH(NAME, iter, map) {
 *         do_something();
 *     } HASHMAP_FOR_EACH_END
 *  It's meant as a feature ...
 * 
 * \param NAME Defined name of map
 * \param ITER _HashType##NAME* denoting the current element.
 * \param MAP Map to iterate over.
 */
#define HASHMAP_FOR_EACH(NAME, ITER, MAP)                                      \
    do {                                                                       \
        if(!(MAP).entries || !(MAP).size) {                                    \
            break;                                                             \
        }                                                                      \
        for(HASMAP_SIZE_T __i = 0, __broke = 0; !__broke &&                           \
                               __i < _##NAME##Primes[(MAP).nth_prime]; ++__i) {\
            if(!(MAP).entries[__i].entries) {                                  \
                continue;                                                      \
            }                                                                  \
            for(HASMAP_SIZE_T __h = 0; !__broke && __h < (MAP).entries[__i].size;     \
                                                                      ++__h) { \
                ITER = &(MAP).entries[__i].entries[__h];                       \
                __broke = 1;                                                   \
                do

/**
 * Closes a HASHMAP_FOR_EACH(...)
 */
#define HASHMAP_FOR_EACH_END                                                   \
                while( __broke = 0, __broke );                                 \
            }                                                                  \
        }                                                                      \
    } while(0);

/**
 * Like HASHMAP_FOR_EACH(ITER, MAP), but you are safe to delete elements during
 * the loop. You deleted elements may or may not show up during the for-loop!
 */
#define HASHMAP_FOR_EACH_SAFE_TO_DELETE(NAME, ITER, MAP)                       \
    do {                                                                       \
        if(!(MAP).entries || !(MAP).size) {                                    \
            break;                                                             \
        }                                                                      \
        for(HASMAP_SIZE_T __i = 0, __broke = 0; !__broke &&                           \
                               __i < _##NAME##Primes[(MAP).nth_prime]; ++__i) {\
            if(!(MAP).entries[__i].entries) {                                  \
                continue;                                                      \
            }                                                                  \
            const HASMAP_SIZE_T __size = map.entries[__i].size;                       \
            _HashType##NAME __entries[__size];                                 \
            memcpy(__entries, &(MAP).entries[__i].entries, sizeof(__entries)); \
            for(HASMAP_SIZE_T __h = 0; !__broke && __h < __size; ++__h) {             \
                ITER = &(MAP).entries[__i].entries[__h];                       \
                __broke = true;                                                \
                do

/**
 * Closes a HASHMAP_FOR_EACH_SAFE_TO_DELETE(...)
 */
#define HASHMAP_FOR_EACH_SAFE_TO_DELETE_END HASHMAP_FOR_EACH_END

/**
 * Declares the hash map functions.
 * \param NAME Typedef'd name of the HashMap type.
 * \param CMP int (*cmp)(_HashType##NAME *left, _HashType##NAME *right).
 *            Could easily be a macro. Must return 0 if and only if *left
 *            equals *right.
 * \param GET_HASH inttype (*getHash)(_HashType##NAME *entry). Could easily be
 *                 a macro.
 * \param FREE free() to use
 * \param REALLOC realloc() to use. Assumes accordance with C standard, i.e.
 *                realloc(NULL, size) behaves as malloc(size).
 */
#define DECLARE_HASHMAP(NAME, CMP, GET_HASH, FREE, REALLOC)                    \
                                                                               \
const HASMAP_SIZE_T _##NAME##Primes[] = { _HASHMAP_PRIMES, 0 };                       \
                                                                               \
void NAME##New(NAME *map) {                                                    \
    map->size = 0;                                                             \
    map->nth_prime = 0;                                                        \
    map->entries = NULL;                                                       \
}                                                                              \
                                                                               \
void NAME##Destroy(NAME *map) {                                                \
    if(map->size && map->entries) {                                            \
        HASMAP_SIZE_T capacity = _##NAME##Primes[map->nth_prime];                     \
        for(HASMAP_SIZE_T i = 0; i < capacity; ++i) {                                 \
            if(map->entries[i].entries) {                                      \
                FREE(map->entries[i].entries);                                 \
            }                                                                  \
        }                                                                      \
    }                                                                          \
    FREE(map->entries);                                                        \
    map->size = 0;                                                             \
    map->nth_prime = 0;                                                        \
    map->entries = NULL;                                                       \
}                                                                              \
                                                                               \
/* Looks for smallest prime p: 2^n < capacity <= p < 2^(n+1)                 */\
/* \param capacity Capacity to ensure.                                       */\
/* \param entries Boolean flag, if nth_prime_ is meaningful.                 */\
/* \param entries nth_prime_ [In/out] current capacity, see _HASHMAP_PRIMES  */\
/* \param newSize_ [Out] p (see description)                                 */\
static _HashMapNextPrimeResult _##NAME##NextPrime(HASMAP_SIZE_T capacity,             \
                                                  const void *entries,         \
                                                  uint8_t *nth_prime_,         \
                                                  HASMAP_SIZE_T *newSize_) {          \
    HASMAP_SIZE_T oldSize = _##NAME##Primes[*nth_prime_];                             \
    if(!capacity || (entries && oldSize >= capacity)) {                        \
        return _HMNPR_NOT_NEEDED;                                              \
    }                                                                          \
    int nth_prime = 0;                                                         \
    HASMAP_SIZE_T newSize;                                                            \
    while((newSize = _##NAME##Primes[nth_prime]) < capacity) {                 \
        if(!newSize) {                                                         \
            return _HMNPR_FAIL;                                                \
        }                                                                      \
        ++nth_prime;                                                           \
    }                                                                          \
    *nth_prime_ = nth_prime;                                                   \
    *newSize_ = newSize;                                                       \
    return _HMNPR_GREW;                                                        \
}                                                                              \
                                                                               \
/* Helper function that puts an entry into the map, with checking the size   */\
/* or minding duplicates.                                                    */\
/* \param map Map to put entry into.                                         */\
/* \param entry Entry to insert in map.                                      */\
/* \return pointer to inserted element, or NULL if could not grow            */\
static _HashType##NAME *_##NAME##PutReal(NAME *map,                            \
                                         _HashType##NAME *entry) {             \
    NAME##Bucket *bucket = &map->entries[((HASMAP_SIZE_T)(GET_HASH(entry))) %         \
                                         _##NAME##Primes[map->nth_prime]];     \
    uint8_t nth_prime = bucket->nth_prime;                                     \
    HASMAP_SIZE_T newSize = 0;                                                        \
    switch(_##NAME##NextPrime(bucket->size+1, bucket->entries, &nth_prime,     \
                                                               &newSize)) {    \
        case _HMNPR_FAIL:                                                      \
            return NULL;                                                       \
        case _HMNPR_GREW:                                                      \
            bucket->entries = REALLOC(bucket->entries,                         \
                                      sizeof(_HashType##NAME[newSize]));       \
            break;                                                             \
        case _HMNPR_NOT_NEEDED:                                                \
            break;                                                             \
        default:                                                               \
            return NULL;                                                       \
    }                                                                          \
    _HashType##NAME *result = &bucket->entries[bucket->size ++];               \
    *result = *entry;                                                          \
    return result;                                                             \
}                                                                              \
                                                                               \
bool NAME##EnsureSize(NAME *map,                                               \
                      HASMAP_SIZE_T capacity) {                                       \
    capacity = (capacity+2)/3 * 4; /* load factor = 0.75 */                    \
    uint8_t nth_prime = map->nth_prime;                                        \
    HASMAP_SIZE_T oldCapacity = _##NAME##Primes[nth_prime];                           \
    HASMAP_SIZE_T newSize = 0;                                                        \
    switch(_##NAME##NextPrime(capacity, map->entries, &nth_prime, &newSize)) { \
        case _HMNPR_FAIL:                                                      \
            return false;                                                      \
        case _HMNPR_NOT_NEEDED:                                                \
            return true;                                                       \
        case _HMNPR_GREW:                                                      \
            break;                                                             \
        default:                                                               \
            return false;                                                      \
    }                                                                          \
    NAME##Bucket *oldEntries = map->entries;                                   \
    NAME##Bucket *newEntries = (NAME##Bucket*) REALLOC(NULL,                   \
                                              sizeof(NAME##Bucket[newSize]));  \
    if(!newEntries) {                                                          \
        return false;                                                          \
    }                                                                          \
    memset(&newEntries[0], 0, sizeof(NAME##Bucket[newSize]));                  \
    map->entries = newEntries;                                                 \
    map->nth_prime = nth_prime;                                                \
    /* TODO: a failed _##NAME##PutReal(...) would corrupt the map! */          \
    if(map->size) {                                                            \
        for(HASMAP_SIZE_T i = 0; i < oldCapacity; ++i) {                              \
            NAME##Bucket *bucket = &oldEntries[i];                             \
            for(HASMAP_SIZE_T h = 0; h < bucket->size; ++h) {                         \
                _##NAME##PutReal(map, &bucket->entries[h]);                    \
            }                                                                  \
            FREE(bucket->entries);                                             \
        }                                                                      \
    }                                                                          \
    FREE(oldEntries);                                                          \
    return true;                                                               \
}                                                                              \
                                                                               \
bool NAME##Find(const NAME *map,                                               \
                _HashType##NAME **entry) {                                     \
    if(!map->entries) {                                                        \
        return NULL;                                                           \
    }                                                                          \
    NAME##Bucket *bucket = &map->entries[((HASMAP_SIZE_T)(GET_HASH((*entry)))) %      \
                                         _##NAME##Primes[map->nth_prime]];     \
    for(HASMAP_SIZE_T h = 0; h < bucket->size; ++h) {                                 \
        if((CMP((&bucket->entries[h]), (*entry))) == 0) {                      \
            *entry = &bucket->entries[h];                                      \
            return true;                                                       \
        }                                                                      \
    }                                                                          \
    return false;                                                              \
}                                                                              \
                                                                               \
HashMapPutResult NAME##Put(NAME *map,                                          \
                           _HashType##NAME **entry,                            \
                           HashMapDuplicateResolution dr) {                    \
    HashMapPutResult result;                                                   \
    _HashType##NAME *current = *entry;                                         \
    if(!NAME##Find(map, &current)) {                                           \
        current = *entry;                                                      \
        result = HMPR_PUT;                                                     \
    } else switch(dr) {                                                        \
        case HMDR_FAIL:                                                        \
            *entry = current;                                                  \
            return HMPR_FAILED;                                                \
        case HMDR_FIND:                                                        \
            *entry = current;                                                  \
            return HMPR_FOUND;                                                 \
        case HMDR_REPLACE: {                                                   \
            *current = **entry;                                                \
            *entry = current;                                                  \
            return HMPR_REPLACED;                                              \
        }                                                                      \
        case HMDR_SWAP: {                                                      \
            _HashType##NAME tmp = *current;                                    \
            *current = **entry;                                                \
            **entry = tmp;                                                     \
            *entry = current;                                                  \
            return HMPR_SWAPPED;                                               \
        }                                                                      \
        case HMDR_STACK: {                                                     \
            _HashType##NAME *tmp = *entry;                                     \
            *entry = current;                                                  \
            current = tmp;                                                     \
            result = HMPR_STACKED;                                             \
            break;                                                             \
        }                                                                      \
        default:                                                               \
            return HMPR_FAILED;                                                \
    }                                                                          \
    if(!NAME##EnsureSize(map, map->size+1)) {                                  \
        return HMPR_FAILED;                                                    \
    }                                                                          \
    _HashType##NAME *putEntry = _##NAME##PutReal(map, current);                \
    if(result == HMPR_PUT) {                                                   \
        *entry = putEntry;                                                     \
    }                                                                          \
    ++map->size;                                                               \
    return result;                                                             \
}                                                                              \
                                                                               \
bool NAME##Remove(NAME *map,                                                   \
                  _HashType##NAME *entry) {                                    \
    NAME##Bucket *bucket = &map->entries[((HASMAP_SIZE_T)(GET_HASH(entry))) %         \
                                         _##NAME##Primes[map->nth_prime]];     \
    for(HASMAP_SIZE_T nth = 0; nth < bucket->size; ++nth) {                           \
         if((CMP(entry, (&bucket->entries[nth]))) == 0) {                      \
            *entry = bucket->entries[nth];                                     \
            memmove(&bucket->entries[nth],                                     \
                    &bucket->entries[nth+1],                                   \
                    sizeof(NAME##Bucket[bucket->size - nth - 1]));             \
            --bucket->size;                                                    \
            return true;                                                       \
        }                                                                      \
    }                                                                          \
    return false;                                                              \
}

#endif // ifndef HASHMAP_H__
