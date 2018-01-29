#ifndef _hashmap_h_
#define _hashmap_h_

#include "dict.h"

/* .h macros */
#define str_t void *
#define double_t double

#define hm_find_h(type)   type##_t hm_##type##_find(HashMap *hm, const void *key)
#define find_ret(dict, key, field) do { \
    dictEntry *entry = dictFind(dict, key); \
    return entry ? entry->v.##field : entry; \
} while (0)

#define hm_insert_h(type) int hm_##type##_insert(HashMap *hm, void *key, type##_t value)
#define insert_not_str(settype) do { \
    dictEntry *entry = dictAddRaw(hm->dt, key); \
    if (entry != NULL) \
    { \
        settype(entry, value); \
        return DICT_OK; \
    } \
    return DICT_ERR; \
} while (0)

#define hm_update_h(type) int hm_##type##_update(HashMap *hm, void *key, type##_t newvalue)
#define update_not_str(settype) do { \
    dictEntry *entry = dictFind(hm->dt, key); \
    if (entry != NULL) \
        settype(entry, newvalue); \
    else  \
        return hm_insert(int64, hm, key, newvalue); \
    return DICT_OK; \
} while (0)

#define hm_delete_h(type) void hm_##type##_delete(HashMap *hm, const void *key)

/* API macros */
#define hm_find(type, hm, key)             hm_##type##_find(hm, key)
#define hm_insert(type, hm, key, value)    hm_##type##_insert(hm, key, value)
#define hm_update(type, hm, key, newvalue) hm_##type##_update(hm, key, newvalue)
#define hm_delete(type, hm, key)           hm_##type##_delete(hm, key)


typedef struct _hashmap
{
    dict *dt;
} HashMap;

HashMap *hm_init();

// str_t hm_str_find(HashMap *hm, const void *key);
// int64_t hm_int64_find(HashMap *hm, const void *key);
// uint64_t hm_uint64_find(HashMap *hm, const void *key);
// double_t hm_double_find(HashMap *hm, const void *key);

hm_find_h(str);
hm_find_h(int64);
hm_find_h(uint64);
hm_find_h(double);

hm_insert_h(str);
hm_insert_h(int64);
hm_insert_h(uint64);
hm_insert_h(double);

hm_update_h(str);
hm_update_h(int64);
hm_update_h(uint64);
hm_update_h(double);

hm_delete_h(str);
hm_delete_h(int64);
hm_delete_h(uint64);
hm_delete_h(double);

void hm_release(HashMap *hm);

// ELF Hash Function
static unsigned int ELFHash(const void *key);

static void *_hashmapStringDup(const void *data);

static int _hashmapKeyCompare(const void *key1, const void *key2);

static void _hashmapStringDestructor(void *data);

#endif