#ifndef _DICT_C
#define _DICT_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <sys/time.h>
#include <ctype.h>

#include "dict.h"

static int dict_can_resize = 1;
static unsigned int dict_force_resize_ratio = 5;

/* -------------------------- private prototypes ---------------------------- */

static int _dictExpandIfNeeded(dict *ht);
static unsigned long _dictNextPower(unsigned long size);
static int _dictKeyIndex(dict *ht, const void *key);
static int _dictInit(dict *ht, dictType *type);

/* -------------------------- hash functions -------------------------------- */

/* Thomas Wang's 32 bit Mix Function */
unsigned int dictIntHashFunction(unsigned int key)
{
    key += ~(key << 15);
    key ^=  (key >> 10);
    key +=  (key << 3);
    key ^=  (key >> 6);
    key += ~(key << 11);
    key ^=  (key >> 16);
    return key;
}

/* Identity hash function for integer keys */
unsigned int dictIdentityHashFunction(unsigned int key)
{
    return key;
}

static uint32_t dict_hash_function_seed = 5381;

void dictSetHashFunctionSeed(uint32_t seed) {
    dict_hash_function_seed = seed;
}

uint32_t dictGetHashFunctionSeed(void) {
    return dict_hash_function_seed;
}

/* MurmurHash2, by Austin Appleby
 * Note - This code makes a few assumptions about how your machine behaves -
 * 1. We can read a 4-byte value from any address without crashing
 * 2. sizeof(int) == 4
 *
 * And it has a few limitations -
 *
 * 1. It will not work incrementally.
 * 2. It will not produce the same results on little-endian and big-endian
 *    machines.
 */
unsigned int dictGenHashFunction(const void *key, int len) {
    /* 'm' and 'r' are mixing constants generated offline.
     They're not really 'magic', they just happen to work well.  */
    uint32_t seed = dict_hash_function_seed;
    const uint32_t m = 0x5bd1e995;
    const int r = 24;

    /* Initialize the hash to a 'random' value */
    uint32_t h = seed ^ len;

    /* Mix 4 bytes at a time into the hash */
    const unsigned char *data = (const unsigned char *)key;

    while(len >= 4) {
        uint32_t k = *(uint32_t*)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    /* Handle the last few bytes of the input array  */
    switch(len) {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0]; h *= m;
    };

    /* Do a few final mixes of the hash to ensure the last few
     * bytes are well-incorporated. */
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return (unsigned int)h;
}

/* And a case insensitive hash function (based on djb hash) */
unsigned int dictGenCaseHashFunction(const unsigned char *buf, int len) {
    unsigned int hash = (unsigned int)dict_hash_function_seed;

    while (len--)
        hash = ((hash << 5) + hash) + (tolower(*buf++)); /* hash * 33 + c */
    return hash;
}

/* ----------------------------- API implementation ------------------------- */

static void _dictReset(dictht *ht)
{
    ht->table = NULL;
    ht->size = 0;
    ht->sizemask = 0;
    ht->used = 0;
}

dict *dictCreate(dictType *type)
{
    dict *d = (struct dict *)malloc(sizeof(*d));
    _dictInit(d,type);
    return d;
}

/* Initialize the hash table */
int _dictInit(dict *d, dictType *type)
{
    _dictReset(&d->ht[0]);
    _dictReset(&d->ht[1]);

    d->type = type;

    d->rehashidx = -1;

    return DICT_OK;
}

/* 缩小给定字典
 * 让它的已用节点数和字典大小之间的比率接近 1:1
 *
 * 返回 DICT_ERR 表示字典已经在 rehash  或者 dict_can_resize 为假
 *
 * 成功创建体积更小的 ht[1]  可以开始 resize 时 返回 DICT_OK
 */
int dictResize(dict *d)
{
    int minimal;

    if (!dict_can_resize || dictIsRehashing(d)) return DICT_ERR;

    minimal = d->ht[0].used;
    if (minimal < DICT_HT_INITIAL_SIZE)
        minimal = DICT_HT_INITIAL_SIZE;

    return dictExpand(d, minimal);
}

/* 创建一个新的哈希表 并根据字典的情况 选择以下其中一个动作来进行：
 *
 * 1) 如果字典的 0 号哈希表为空 那么将新哈希表设置为 0 号哈希表
 * 2) 如果字典的 0 号哈希表非空 那么将新哈希表设置为 1 号哈希表 
 *    并打开字典的 rehash 标识 使得程序可以开始对字典进行 rehash
 *
 * size 参数不够大 或者 rehash 已经在进行时 返回 DICT_ERR 
 *
 * 成功创建 0 号哈希表 或者 1 号哈希表时 返回 DICT_OK 
 */
int dictExpand(dict *d, unsigned long size)
{
    dictht n; /* the new hash table */

    unsigned long realsize = _dictNextPower(size);

    if (dictIsRehashing(d) || d->ht[0].used > size)
        return DICT_ERR;

    n.size = realsize;
    n.sizemask = realsize-1;
    n.table = (struct dictEntry **)calloc(realsize, sizeof(dictEntry*));
    n.used = 0;

    if (d->ht[0].table == NULL) {
        d->ht[0] = n;
    }
    else
    {
        d->ht[1] = n;
        d->rehashidx = 0;
    }

    return DICT_OK;
}

/* 执行 N 步渐进式 rehash 
 *
 * 返回 1 表示仍有键需要从 0 号哈希表移动到 1 号哈希表 
 * 返回 0 则表示所有键都已经迁移完毕
 */
int dictRehash(dict *d, int n) {
    if (!dictIsRehashing(d)) return 0;

    while(n--) {
        dictEntry *de, *nextde;

        if (d->ht[0].used == 0) {
            free(d->ht[0].table);
            d->ht[0] = d->ht[1];
            _dictReset(&d->ht[1]);
            d->rehashidx = -1;
            return 0;
        }

        while(d->ht[0].table[d->rehashidx] == NULL) d->rehashidx++;

        de = d->ht[0].table[d->rehashidx];
        while(de) {
            unsigned int h;
            nextde = de->next;

            /* Get the index in the new hash table */
            h = dictHashKey(d, de->key) & d->ht[1].sizemask;

            de->next = d->ht[1].table[h];
            d->ht[1].table[h] = de;

            d->ht[0].used--;
            d->ht[1].used++;

            de = nextde;
        }
        d->ht[0].table[d->rehashidx] = NULL;
        d->rehashidx++;
    }

    return 1;
}

static void _dictRehashStep(dict *d) {
    dictRehash(d,1);
}

int dictAdd(dict *d, const void *key, const void *val)
{
    dictEntry *entry = dictAddRaw(d,key);
    if (!entry) return DICT_ERR;
    dictSetVal(d, entry, val);
    return DICT_OK;
}

/*
 * 尝试将键插入到字典中
 * 如果键已经在字典存在 那么返回 NULL
 * 如果键不存在 那么程序创建新的哈希节点 
 * 将节点和键关联 并插入到字典 然后返回节点本身
 */
dictEntry *dictAddRaw(dict *d, const void *key)
{
    int index;
    dictEntry *entry;
    dictht *ht;

    if (dictIsRehashing(d)) _dictRehashStep(d);

    if ((index = _dictKeyIndex(d, key)) == -1)
        return NULL;

    ht = dictIsRehashing(d) ? &d->ht[1] : &d->ht[0];
    entry = (struct dictEntry *)malloc(sizeof(*entry));
    entry->next = ht->table[index]; /* Insert to the top of list */
    ht->table[index] = entry;
    ht->used++;

    dictSetKey(d, entry, key);

    return entry;
}

/*
 * 将给定的键值对添加到字典中 如果键已经存在 那么删除旧有的键值对
 *
 * 如果键值对为全新添加 那么返回 1 
 * 如果键值对是通过对原有的键值对更新得来的 那么返回 0 
 */
int dictReplace(dict *d, const void *key, const void *val)
{
    dictEntry *entry, auxentry;

    if (dictAdd(d, key, val) == DICT_OK)
        return 1;

    /* It already exists, get the entry */
    entry = dictFind(d, key);
    /* Set the new value and free the old one. Note that it is important
     * to do that in this order, as the value may just be exactly the same
     * as the previous one. In this context, think to reference counting,
     * you want to increment (set), and then decrement (free), and not the
     * reverse. */
    auxentry = *entry;
    dictSetVal(d, entry, val);
    dictFreeVal(d, &auxentry);

    return 0;
}

/*
 * dictAddRaw() 根据给定 key 释放存在 执行以下动作：
 *
 * 1) key 已经存在 返回包含该 key 的字典节点
 * 2) key 不存在 那么将 key 添加到字典
 *
 * 不论发生以上的哪一种情况 
 * dictAddRaw() 都总是返回包含给定 key 的字典节点
 */
dictEntry *dictReplaceRaw(dict *d, const void *key) {
    dictEntry *entry = dictFind(d,key);
    return entry ? entry : dictAddRaw(d,key);
}

/* Search and remove an element
 * 参数 nofree 决定是否调用键和值的释放函数
 * 0 表示调用 1 表示不调用
 */
static int dictGenericDelete(dict *d, const void *key, int nofree)
{
    unsigned int h, idx;
    dictEntry *he, *prevHe;
    int table;

    if (d->ht[0].size == 0) return DICT_ERR; /* d->ht[0].table is NULL */

    if (dictIsRehashing(d)) _dictRehashStep(d);

    h = dictHashKey(d, key);

    for (table = 0; table <= 1; table++) {
        idx = h & d->ht[table].sizemask;
        he = d->ht[table].table[idx];
        prevHe = NULL;
        while(he) {
            if (dictCompareKeys(d, key, he->key)) {
                /* Unlink the element from the list */
                if (prevHe)
                    prevHe->next = he->next;
                else
                    d->ht[table].table[idx] = he->next;

                if (!nofree) {
                    dictFreeVal(d, he);
                }
                
                dictFreeKey(d, he);
                free(he);
                d->ht[table].used--;

                return DICT_OK;
            }

            prevHe = he;
            he = he->next;
        }

        if (!dictIsRehashing(d)) break;
    }

    return DICT_ERR; /* not found */
}

int dictDelete(dict *ht, const void *key) {
    return dictGenericDelete(ht,key,0);
}

int dictDeleteNoFree(dict *ht, const void *key) {
    return dictGenericDelete(ht,key,1);
}

int _dictClear(dict *d, dictht *ht) {
    unsigned long i;

    for (i = 0; i < ht->size && ht->used > 0; i++) {
        dictEntry *he, *nextHe;
        if ((he = ht->table[i]) == NULL) continue;

        while(he) {
            nextHe = he->next;
            dictFreeKey(d, he);
            dictFreeVal(d, he);
            free(he);

            ht->used--;
            he = nextHe;
        }
    }

    free(ht->table);
    _dictReset(ht);
    return DICT_OK;
}

void dictRelease(dict *d)
{
    _dictClear(d,&d->ht[0]);
    _dictClear(d,&d->ht[1]);
    free(d);
}

dictEntry *dictFind(dict *d, const void *key)
{
    dictEntry *he;
    unsigned int h, idx, table;

    if (d->ht[0].size == 0) return NULL; /* We don't have a table at all */

    if (dictIsRehashing(d)) _dictRehashStep(d);

    h = dictHashKey(d, key);
    for (table = 0; table <= 1; table++) {
        idx = h & d->ht[table].sizemask;
        he = d->ht[table].table[idx];
        while(he) {
            if (dictCompareKeys(d, key, he->key))
                return he;

            he = he->next;
        }
        if (!dictIsRehashing(d)) return NULL;
    }

    return NULL;
}

/*
 * 获取包含给定键的节点的值
 *
 * 如果节点不为空 返回节点的值
 * 否则返回 NULL
 */
void *dictFetchValue(dict *d, const void *key) {
    dictEntry *he;

    he = dictFind(d,key);

    return he ? (void *)dictGetVal(he) : NULL;
}

/* Function to reverse bits. Algorithm from:
 * http://graphics.stanford.edu/~seander/bithacks.html#ReverseParallel */
static unsigned long rev(unsigned long v) {
    unsigned long s = 8 * sizeof(v); // bit size; must be power of 2
    unsigned long mask = ~0;
    while ((s >>= 1) > 0) {
        mask ^= (mask << s);
        v = ((v >> s) & mask) | ((v << s) & ~mask);
    }
    return v;
}


/* ------------------------- private functions ------------------------------ */

/* Expand the hash table if needed */
static int _dictExpandIfNeeded(dict *d)
{
    if (dictIsRehashing(d)) return DICT_OK;

    if (d->ht[0].size == 0) return dictExpand(d, DICT_HT_INITIAL_SIZE);

    if (d->ht[0].used >= d->ht[0].size &&
        (dict_can_resize ||
         d->ht[0].used/d->ht[0].size > dict_force_resize_ratio))
    {
        return dictExpand(d, d->ht[0].used*2);
    }

    return DICT_OK;
}

/* Our hash table capability is a power of two */
static unsigned long _dictNextPower(unsigned long size)
{
    unsigned long i = DICT_HT_INITIAL_SIZE;

    if (size >= LONG_MAX) return LONG_MAX;
    while(1) {
        if (i >= size)
            return i;
        i *= 2;
    }
}

/*
 * 返回可以将 key 插入到哈希表的索引位置
 * 如果 key 已经存在于哈希表 那么返回 -1
 *
 * 注意 如果字典正在进行 rehash  那么总是返回 1 号哈希表的索引
 * 因为在字典进行 rehash 时 新节点总是插入到 1 号哈希表
 */
static int _dictKeyIndex(dict *d, const void *key)
{
    unsigned int h, idx, table;
    dictEntry *he;

    /* Expand the hash table if needed */
    if (_dictExpandIfNeeded(d) == DICT_ERR)
        return -1;

    /* Compute the key hash value */
    h = dictHashKey(d, key);
    for (table = 0; table <= 1; table++) {
        idx = h & d->ht[table].sizemask;

        /* Search if this slot does not already contain the given key */
        he = d->ht[table].table[idx];
        while(he) {
            if (dictCompareKeys(d, key, he->key))
                return -1;
            he = he->next;
        }

        if (!dictIsRehashing(d)) break;
    }

    return idx;
}

void dictEmpty(dict *d) {
    _dictClear(d,&d->ht[0]);
    _dictClear(d,&d->ht[1]);
    d->rehashidx = -1;
}

void dictEnableResize(void) {
    dict_can_resize = 1;
}

void dictDisableResize(void) {
    dict_can_resize = 0;
}

#if 0

/* The following is code that we don't use for Redis currently, but that is part
of the library. */

/* ----------------------- Debugging ------------------------*/

#define DICT_STATS_VECTLEN 50
static void _dictPrintStatsHt(dictht *ht) {
    unsigned long i, slots = 0, chainlen, maxchainlen = 0;
    unsigned long totchainlen = 0;
    unsigned long clvector[DICT_STATS_VECTLEN];

    if (ht->used == 0) {
        printf("No stats available for empty dictionaries\n");
        return;
    }

    for (i = 0; i < DICT_STATS_VECTLEN; i++) clvector[i] = 0;
    for (i = 0; i < ht->size; i++) {
        dictEntry *he;

        if (ht->table[i] == NULL) {
            clvector[0]++;
            continue;
        }
        slots++;
        /* For each hash entry on this slot... */
        chainlen = 0;
        he = ht->table[i];
        while(he) {
            chainlen++;
            he = he->next;
        }
        clvector[(chainlen < DICT_STATS_VECTLEN) ? chainlen : (DICT_STATS_VECTLEN-1)]++;
        if (chainlen > maxchainlen) maxchainlen = chainlen;
        totchainlen += chainlen;
    }
    printf("Hash table stats:\n");
    printf(" table size: %ld\n", ht->size);
    printf(" number of elements: %ld\n", ht->used);
    printf(" different slots: %ld\n", slots);
    printf(" max chain length: %ld\n", maxchainlen);
    printf(" avg chain length (counted): %.02f\n", (float)totchainlen/slots);
    printf(" avg chain length (computed): %.02f\n", (float)ht->used/slots);
    printf(" Chain length distribution:\n");
    for (i = 0; i < DICT_STATS_VECTLEN-1; i++) {
        if (clvector[i] == 0) continue;
        printf("   %s%ld: %ld (%.02f%%)\n",(i == DICT_STATS_VECTLEN-1)?">= ":"", i, clvector[i], ((float)clvector[i]/ht->size)*100);
    }
}

void dictPrintStats(dict *d) {
    _dictPrintStatsHt(&d->ht[0]);
    if (dictIsRehashing(d)) {
        printf("-- Rehashing into ht[1]:\n");
        _dictPrintStatsHt(&d->ht[1]);
    }
}

/* ----------------------- StringCopy Hash Table Type ------------------------*/

static unsigned int _dictStringCopyHTHashFunction(const void *key)
{
    return dictGenHashFunction(key, strlen(key));
}

static void *_dictStringDup(void *privdata, const void *key)
{
    int len = strlen(key);
    char *copy = zmalloc(len+1);
    DICT_NOTUSED(privdata);

    memcpy(copy, key, len);
    copy[len] = '\0';
    return copy;
}

static int _dictStringCopyHTKeyCompare(void *privdata, const void *key1,
        const void *key2)
{
    DICT_NOTUSED(privdata);

    return strcmp(key1, key2) == 0;
}

static void _dictStringDestructor(void *privdata, void *key)
{
    DICT_NOTUSED(privdata);

    free(key);
}

dictType dictTypeHeapStringCopyKey = {
    _dictStringCopyHTHashFunction, /* hash function */
    _dictStringDup,                /* key dup */
    NULL,                          /* val dup */
    _dictStringCopyHTKeyCompare,   /* key compare */
    _dictStringDestructor,         /* key destructor */
    NULL                           /* val destructor */
};

/* This is like StringCopy but does not auto-duplicate the key.
 * It's used for intepreter's shared strings. */
dictType dictTypeHeapStrings = {
    _dictStringCopyHTHashFunction, /* hash function */
    NULL,                          /* key dup */
    NULL,                          /* val dup */
    _dictStringCopyHTKeyCompare,   /* key compare */
    _dictStringDestructor,         /* key destructor */
    NULL                           /* val destructor */
};

/* This is like StringCopy but also automatically handle dynamic
 * allocated C strings as values. */
dictType dictTypeHeapStringCopyKeyValue = {
    _dictStringCopyHTHashFunction, /* hash function */
    _dictStringDup,                /* key dup */
    _dictStringDup,                /* val dup */
    _dictStringCopyHTKeyCompare,   /* key compare */
    _dictStringDestructor,         /* key destructor */
    _dictStringDestructor,         /* val destructor */
};
#endif

#endif