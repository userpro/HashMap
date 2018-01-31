#include <stdio.h>
#include "hashmap.h"

void test(HashMap mm)
{
    hm_set(mm, "QQQ", 22, int32);
}

int main()
{
    HashMap mm = hm_init();
    hm_set(mm, "QAQ", 3.14, double);
    hm_get(mm, "QAQ", double);
    hm_update(mm, "QAQ", 6.28, double);
    hm_get(mm, "QAQ", double);
    
    hm_set(mm, "QWQ", "hello world", HM_STR);
    printf("%s\n", hm_get(mm, "QWQ", HM_STR));

    test(mm);
    printf("%d\n", hm_get(mm, "QQQ", int32));

    hm_release(mm);
    return 0;
}