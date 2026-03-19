//
// Created by bgd on 2025/2/10.
//

#include <string.h>

#include "util.h"
#include "plat/alloc.h"
#include "plat/util.h"

uint64_t unix_timestamp_ms()
{
    return plat_unix_timestamp_ms();
}

uint64_t unix_timestamp()
{
    return plat_unix_timestamp();
}

int32_t random_num()
{
    return plat_random_num();
}

struct aws_allocator *aws_alloc(void)
{
    return plat_aws_alloc();
}