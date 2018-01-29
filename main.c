#include <stdio.h>
#include "hashmap.h"

int main(int argc, char const *argv[])
{
    HashMap *m = hm_init();
    char key1[] = "hi";
    char a[] = "wocao";
    char c[] = "fkfkfkf";
    printf("insert code: %d\n", hm_insert(str, m, key1, a));
    printf("hi: %s\n", hm_find(str, m, key1));
    hm_str_update(m, key1, c);
    printf("hi: %s\n", hm_find(str, m, key1));

    int na = 10;
    hm_insert(int64, m, "kkk", na);
    printf("kkk: %d\n", hm_find(int64, m, "kkk"));
    hm_update(int64, m, "kkk", 20);
    printf("kkk: %d\n", hm_find(int64, m, "kkk"));
    hm_delete(int64, m, "kkk");
    printf("kkk: %d\n", hm_find(int64, m, "kkk"));

    hm_release(m);
    return 0;
}