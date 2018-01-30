#include <stdio.h>
#include "hashmap.h"

int main(int argc, char const *argv[])
{
    HashMap m = hm_init();

    // string
    printf("insert code: %d\n", hm_set(m, "hi", "wocao", hm_str)); // 0=>OK  1=>ERR
    printf("hi: %s\n", hm_get(m, "hi", hm_str));
    hm_update(m, "hi", "fkfkfkf", hm_str);
    printf("hi: %s\n", hm_get(m, "hi", hm_str));

    // int32
    hm_set(m, "kkk", 10, int32);
    printf("kkk: %d\n", hm_get(m, "kkk", int32));
    hm_update(m, "kkk", 20, int32);
    printf("kkk: %d\n", hm_get(m, "kkk", int32));
    hm_del(m, "kkk", int32);
    printf("kkk: %d\n", hm_get(m, "kkk", int32));

    // uint32
    hm_set(m, "qwq", 1e9+888, uint32);
    printf("qwq: %u\n", hm_get(m, "qwq", uint32));
    hm_update(m, "qwq", 2e8-23193, uint32);
    printf("qwq: %u\n", hm_get(m, "qwq", uint32));

    // int64
    hm_set(m, "waw", 1e15+55, int64);
    printf("waw: %lld\n", hm_get(m, "waw", int64));
    hm_update(m, "waw", 2e15, int64);
    printf("waw: %lld\n", hm_get(m, "waw", int64));

    // double
    hm_set(m, "vav", 3.14, double);
    printf("vav: %f\n", hm_get(m, "vav", double));
    hm_update(m, "vav", 6.28, double);
    printf("vav: %f\n", hm_get(m, "vav", double));

    // pointer
    int tt = 101;
    int *iptr = &tt;
    hm_set(m, "TAT", iptr, hm_ptr);
    printf("%d\n", *(int *)hm_get(m, "TAT", hm_ptr));

    // simple
    HashMap mm = hm_init();
    hm_set(mm, "QAQ", 3.14, double);
    hm_get(mm, "QAQ", double);
    hm_update(mm, "QAQ", 6.28, double);
    hm_get(mm, "QAQ", double);
    hm_release(mm);

    hm_release(m);
    return 0;
}