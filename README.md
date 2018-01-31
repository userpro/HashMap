# hashmap
一个基于redis中字典类型封装得来的hashmap (稍稍改了点 emmm...)

键类型为字符串

值类型可以为字符串 整数 浮点数 指针

## Example

~~~c
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
~~~

详见test.c

## API
>预定义类型宏(value支持类型)
>>* `hm_ptr` 指针
>>
>>* `hm_str` 字符串
>>
>>* `int32` 32位有符号整数
>>
>>* `uint32` 32位无符号整数
>>
>>* `int64` 64位有符号整数
>>
>>* `uint64` 64位无符号整数
>>
>>* `double` 双精度浮点型

`HashMap` 
>哈希表指针类型

`hm_init()`
>返回值 => 哈希表指针

`hm_set(哈希表指针, 键(字符串类型), 值, 值的类型)`
>返回值 =>  0:插入成功 1:插入失败

`hm_get(哈希表指针, 键(字符串类型), 值)`
>返回值 => 值(若为字符串返回的是指针)
>
>当get键不存在时 对于指针类型返回NULL 其他返回类型最大值

`hm_update(哈希表指针, 键(字符串类型), 新的值, 值的类型)`
>返回值 =>  0:插入成功 1:插入失败

`hm_del(哈希表指针, 键(字符串类型))`
>返回值 => 无

## Usage
`gcc yourfile.c hashmap.c dict.c` or `g++ yourfile.c hashmap.c dict.c`