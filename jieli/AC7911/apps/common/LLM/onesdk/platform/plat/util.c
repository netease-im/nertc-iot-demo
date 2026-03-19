#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include "util.h"


uint64_t plat_unix_timestamp_ms()   // 函数名修改为 _ms 以表示毫秒
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000);
}

// 保留原来的秒级时间戳函数，如果其他地方还在使用的话
uint64_t plat_unix_timestamp()
{
    return (uint64_t) time(NULL);
}

int32_t plat_random_num()
{
    srand((unsigned) plat_unix_timestamp());
    return rand();
}

