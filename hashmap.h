#ifndef _hashmap_h_
#define _hashmap_h_

#include "dict.h"

/* .h macros */
#define hm_str_t const void *
#define double_t double

/* Get */
#define hm_get_h(type)   type##_t hm_##type##_get(HashMap hm, const char * key)
/* for dict structure */
#define field_str entry->v.val
#define field_u64 entry->v.u64
#define field_s64 entry->v.s64
#define field_d   entry->v.d
#define get_ret(dict, key, field, ret) do { \
    dictEntry *entry = dictFind(dict, key); \
    return entry ? field : ret; \
} while (0)

/* Set */
#define hm_set_h(type) int hm_##type##_set(HashMap hm, const char *key, type##_t value)
#define set_not_str(settype) do { \
    dictEntry *entry = dictAddRaw(hm->dt, key); \
    if (entry != NULL) \
    { \
        settype(entry, value); \
        return DICT_OK; \
    } \
    return DICT_ERR; \
} while (0)

/* Update */
#define hm_update_h(type) int hm_##type##_update(HashMap hm, const char *key, type##_t newvalue)
#define update_not_str(settype) do { \
    dictEntry *entry = dictFind(hm->dt, key); \
    if (entry != NULL) \
        settype(entry, newvalue); \
    else  \
        return hm_set(hm, key, newvalue, int64); \
    return DICT_OK; \
} while (0)

/* Delete */
#define hm_del_h(type) void hm_##type##_del(HashMap hm, const char *key)

/* API macros */
#define hm_get(hm, key, type)              hm_##type##_get(hm, key)
#define hm_set(hm, key, value, type)       hm_##type##_set(hm, key, value)
#define hm_update(hm, key, newvalue, type) hm_##type##_update(hm, key, newvalue)
#define hm_del(hm, key, type)              hm_##type##_del(hm, key)


typedef struct _hashmap
{
    dict *dt;
} _HashMap;
typedef _HashMap* HashMap; /* Basic Type */

HashMap hm_init();

hm_get_h(hm_str);
hm_get_h(int32);
hm_get_h(uint32);
hm_get_h(int64);
hm_get_h(uint64);
hm_get_h(double);

hm_set_h(hm_str);
hm_set_h(int32);
hm_set_h(uint32);
hm_set_h(int64);
hm_set_h(uint64);
hm_set_h(double);

hm_update_h(hm_str);
hm_update_h(int32);
hm_update_h(uint32);
hm_update_h(int64);
hm_update_h(uint64);
hm_update_h(double);

hm_del_h(hm_str);
hm_del_h(int32);
hm_del_h(uint32);
hm_del_h(int64);
hm_del_h(uint64);
hm_del_h(double);

void hm_release(HashMap hm);

static unsigned int _hashmapELFHash(const void *key);

static void *_hashmapStringDup(const void *data);

static int _hashmapKeyCompare(const void *key1, const void *key2);

static void _hashmapStringDestructor(const void *data);

#endif