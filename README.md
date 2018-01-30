# hashmap
	一个基于redis中字典类型封装得来的hashmap (稍稍改了点 emmm...)

键类型为字符串

值类型可以为字符串 整数 浮点数

## Example

~~~c
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
~~~

## API
>预定义类型宏
>>*`hm_str` 字符串 
>>
>>*`int32` 32位有符号整数
>>
>>*`uint32` 32位无符号整数
>>
>>*`int64` 64位有符号整数
>>
>>*`uint64` 64位无符号整数
>>
>>*`double` 双精度浮点型

`HashMap` 
>哈希表指针类型

`hm_init()`
>返回值 => 哈希表指针

`hm_set(哈希表指针, 键(字符串类型), 值, 值的类型)`
>返回值 =>  0:插入成功 1:插入失败

`hm_get(哈希表指针, 键(字符串类型), 值)`
>返回值 => 值(若为字符串返回的是指针)

`hm_update(哈希表指针, 键(字符串类型), 新的值, 值的类型)`
>返回值 =>  0:插入成功 1:插入失败

`hm_del(哈希表指针, 键(字符串类型))`
>返回值 => 无

##Usage
`gcc yourfile.c hashmap.c dict.c` or `g++ yourfile.c hashmap.c dict.c`