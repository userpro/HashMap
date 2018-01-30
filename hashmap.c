#ifndef _hashmap_c_
#define _hashmap_c_

#include <float.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.h"

static unsigned int _hashmapELFHash(const void *key)
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
    int len = strlen((char *)data);
    char *copy = (char *)malloc(len+1);

    memcpy(copy, data, len);
    copy[len] = '\0';
    return copy;
}

static int _hashmapKeyCompare(const void *key1, const void *key2)
{
    return strcmp((char *)key1, (char *)key2) == 0;
}

static void _hashmapKVDestructor(const void *data)
{
    free((void *)data);
}


hm_get_h(HM_STR) { get_ret(hm->dt, key, field_str, NULL);        }
hm_get_h(HM_PTR) { get_ret(hm->dt, key, field_ptr, NULL);        }
hm_get_h(int32)  { get_ret(hm->dt, key, field_s64, INT_MAX);     }
hm_get_h(uint32) { get_ret(hm->dt, key, field_u64, UINT_MAX);    }
hm_get_h(int64)  { get_ret(hm->dt, key, field_s64, LONG_MAX);    }
hm_get_h(uint64) { get_ret(hm->dt, key, field_u64, ULONG_MAX);   }
hm_get_h(double) { get_ret(hm->dt, key, field_d, DBL_MAX);       }

hm_set_h(HM_STR) { return dictAdd(hm->dt, key, value);           }
hm_set_h(HM_PTR) { return dictAdd(hm->dt, key, value);           }
hm_set_h(int32)  { set_not_str(dictSetUnsignedIntegerVal);       }
hm_set_h(uint32) { set_not_str(dictSetUnsignedIntegerVal);       }
hm_set_h(int64)  { set_not_str(dictSetUnsignedIntegerVal);       }
hm_set_h(uint64) { set_not_str(dictSetUnsignedIntegerVal);       }
hm_set_h(double) { set_not_str(dictSetDoubleVal);                }

hm_update_h(HM_STR) { return dictReplace(hm->dt, key, newvalue); }
hm_update_h(HM_PTR) { return dictReplace(hm->dt, key, newvalue); }
hm_update_h(int32)  { update_not_str(dictSetSignedIntegerVal);   }
hm_update_h(int64)  { update_not_str(dictSetSignedIntegerVal);   }
hm_update_h(uint32) { update_not_str(dictSetUnsignedIntegerVal); }
hm_update_h(uint64) { update_not_str(dictSetUnsignedIntegerVal); }
hm_update_h(double) { update_not_str(dictSetDoubleVal);          }

hm_del_h(HM_STR) { dictDelete(hm->dt, key);       }
hm_del_h(HM_PTR) { dictDelete(hm->dt, key);       }
hm_del_h(int32)  { dictDeleteNoFree(hm->dt, key); }
hm_del_h(uint32) { dictDeleteNoFree(hm->dt, key); }
hm_del_h(int64)  { dictDeleteNoFree(hm->dt, key); }
hm_del_h(uint64) { dictDeleteNoFree(hm->dt, key); }
hm_del_h(double) { dictDeleteNoFree(hm->dt, key); }

void hm_release(HashMap hm)
{
    dictRelease(hm->dt);
    free(hm);
}

HashMap hm_init()
{
    dictType *dtype = (dictType *)malloc(sizeof(dictType));
    dtype->hashFunction    = _hashmapELFHash;          /* hash function */
    dtype->keyDup          = _hashmapStringDup;        /* key dup */
    dtype->valDup          = _hashmapStringDup;        /* val dup */
    dtype->keyCompare      = _hashmapKeyCompare;       /* key compare */
    dtype->keyDestructor   = _hashmapKVDestructor; /* key destructor */
    dtype->keyDestructor   = _hashmapKVDestructor; /* val destructor */

    HashMap hm = (HashMap)malloc(sizeof(_HashMap));
    hm->dt = dictCreate(dtype);
    return hm;
}

#endif