#ifndef __PLATFORM_UTIL_H
#define __PLATFORM_UTIL_H

#include <stddef.h>
#include <stdint.h>

uint64_t plat_unix_timestamp_ms();
uint64_t plat_unix_timestamp();
int32_t plat_random_num();

#endif // __PLATFORM_UTIL_H