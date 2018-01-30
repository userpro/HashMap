#include <stdio.h>
#include "hashmap.h"

int main()
{
    HashMap mm = hm_init();
    hm_set(mm, "QAQ", 3.14, double);
    hm_get(mm, "QAQ", double);
    hm_update(mm, "QAQ", 6.28, double);
    hm_get(mm, "QAQ", double);
    
    hm_set(mm, "QWQ", "hello world", hm_str);
    printf("%s\n", hm_get(mm, "QWQ", hm_str));

    hm_release(mm);
    return 0;
}