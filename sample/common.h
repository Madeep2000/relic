#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <time.h>

#define PERFORMANCE_TEST(pre_str, func, count)                                             \
{                                                                                          \
    double time = clock();                                                                 \
    for (size_t i = 0; i < count; i++)                                                     \
    {                                                                                      \
        func;                                                                              \
    }                                                                                      \
    time = clock() - time;                                                                 \
    printf("%s: 运行次数 %ld, 总运行时间 %.7f 秒, 单次运行时间 %.7f 秒, 一秒运行 %.7f 次. \n", \
            pre_str,                                                                       \
            count,                                                                         \
            (time) / CLOCKS_PER_SEC,                                                       \
            (time) / CLOCKS_PER_SEC / count,                                               \
            1.0 / ((time) / CLOCKS_PER_SEC / count));                                      \
}                                                                                          \
    
// 打印字节信息：pre_str: 十六进制的字节形式
void print_hex(char pre_str[], unsigned char bytes[], size_t len);

#endif COMM ON_H
