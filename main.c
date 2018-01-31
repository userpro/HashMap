#include <stdio.h>
#include "hashmap.h"

int main(int argc, char const *argv[])
{
    HashMap m = hm_init();

    // string
    printf("--- String ---\n");
    printf("insert code: %d\n", hm_set(m, "hi", "wocao", HM_STR)); // 0=>OK  1=>ERR
    printf("[set] hi: %s\n", hm_get(m, "hi", HM_STR));
    hm_update(m, "hi", "fkfkfkf", HM_STR);
    printf("[update] hi: %s\n", hm_get(m, "hi", HM_STR));

    // int32
    printf("--- int32 ---\n");
    hm_set(m, "kkk", 10, int32);
    printf("[set] kkk: %d\n", hm_get(m, "kkk", int32));
    hm_update(m, "kkk", -20, int32);
    printf("[update] kkk: %d\n", hm_get(m, "kkk", int32));
    hm_del(m, "kkk", int32);
    printf("[null] kkk: %d\n", hm_get(m, "kkk", int32));

    // uint32
    printf("--- uint32 ---\n");
    hm_set(m, "qwq", 1e9+888, uint32);
    printf("[set] qwq: %u\n", hm_get(m, "qwq", uint32));
    hm_update(m, "qwq", 2e8-23193, uint32);
    printf("[update] qwq: %u\n", hm_get(m, "qwq", uint32));

    // int64
    printf("--- int64 ---\n");
    hm_set(m, "waw", 1e15+55, int64);
    printf("[set] waw: %lld\n", hm_get(m, "waw", int64));
    hm_update(m, "waw", 2e15, int64);
    printf("[update] waw: %lld\n", hm_get(m, "waw", int64));

    // double
    printf("--- double ---\n");
    hm_set(m, "vav", 3.14, double);
    printf("[set] vav: %f\n", hm_get(m, "vav", double));
    hm_update(m, "vav", 6.28, double);
    printf("[update] vav: %f\n", hm_get(m, "vav", double));

    // pointer
    printf("--- pointer ---\n");
    int tt = 101;
    int *iptr = &tt;
    hm_set(m, "TAT", iptr, HM_PTR);
    printf("[set] iptr: %d\n", *(int *)hm_get(m, "TAT", HM_PTR));

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