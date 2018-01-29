#ifndef _hashmap_c_
#define _hashmap_c_

#include <float.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.h"

static unsigned int ELFHash(const void *key)
{
    char *str = (char *)key;
    unsigned int hash=0;
    unsigned int x=0;
    while(*str)
    {
        hash=(hash<<4)+(*str++);
        if((x=hash&0xF0000000L)!=0)
        {
            hash^=(x>>24);
            hash&=~x;
        }
    }
    return hash;
}

static void *_hashmapStringDup(const void *data)
{
    int len = strlen(data);
    char *copy = (char *)malloc(len+1);

    memcpy(copy, data, len);
    copy[len] = '\0';
    return copy;
}

static int _hashmapKeyCompare(const void *key1, const void *key2)
{
    return strcmp(key1, key2) == 0;
}

static void _hashmapStringDestructor(void *data)
{
    free(data);
}


hm_find_h(str) {
    dictEntry *entry = dictFind(hm->dt, key);
    return entry ? entry->v.val : entry;
}
hm_find_h(int64) {
    dictEntry *entry = dictFind(hm->dt, key); 
    return entry ? entry->v.s64 : INT_MAX;
}
hm_find_h(uint64) {
    dictEntry *entry = dictFind(hm->dt, key);
    return entry ? entry->v.u64 : UINT_MAX;
}
hm_find_h(double) {
    dictEntry *entry = dictFind(hm->dt, key);
    return entry ? entry->v.d : DBL_MAX;
}

hm_insert_h(str)    { return dictAdd(hm->dt, key, value);        }
hm_insert_h(int64)  { insert_not_str(dictSetUnsignedIntegerVal); }
hm_insert_h(uint64) { insert_not_str(dictSetUnsignedIntegerVal); }
hm_insert_h(double) { insert_not_str(dictSetDoubleVal);          }

hm_update_h(str)    { return dictReplace(hm->dt, key, newvalue); }
hm_update_h(int64)  { update_not_str(dictSetSignedIntegerVal);   }
hm_update_h(uint64) { update_not_str(dictSetUnsignedIntegerVal); }
hm_update_h(double) { update_not_str(dictSetDoubleVal);          }

hm_delete_h(str)    { dictDelete(hm->dt, key);       }
hm_delete_h(int64)  { dictDeleteNoFree(hm->dt, key); }
hm_delete_h(uint64) { dictDeleteNoFree(hm->dt, key); }
hm_delete_h(double) { dictDeleteNoFree(hm->dt, key); }

void hm_release(HashMap *hm)
{
    dictRelease(hm->dt);
    free(hm);
}

HashMap *hm_init()
{
    dictType *dtype = (dictType *)malloc(sizeof(dictType));
    dtype->hashFunction    = ELFHash;              /* hash function */
    dtype->keyDup          = _hashmapStringDup;    /* key dup */
    dtype->valDup          = _hashmapStringDup;    /* val dup */
    dtype->keyCompare      = _hashmapKeyCompare;   /* key compare */
    dtype->keyDestructor   = _hashmapStringDestructor; /* key destructor */
    dtype->keyDestructor   = _hashmapStringDestructor; /* val destructor */

    HashMap *hm = (HashMap *)malloc(sizeof(HashMap));
    hm->dt = dictCreate(dtype);
    return hm;
}

#endif