#include "volc_time.h"

//#include <stdio.h>
#include <time.h>
//#include <string.h>
#include <sys/time.h>
#include "volc_errno.h"
#include "volc_type.h"


uint64_t volc_get_time_ms(void)
{
    struct timespec now_time;
    clock_gettime(CLOCK_REALTIME, &now_time);
    return (uint64_t)now_time.tv_sec * VOLC_MILLISECONDS_IN_A_SECOND + (uint64_t)now_time.tv_nsec / VOLC_NANOS_IN_A_MILLISECONDS_SECOND;
}

#define CLOCK_MONOTONIC 4
uint64_t volc_get_montionic_time_ms(void)
{
    struct timespec now_time;
    clock_gettime(CLOCK_MONOTONIC, &now_time);
    return (uint64_t)now_time.tv_sec * VOLC_MILLISECONDS_IN_A_SECOND + (uint64_t)now_time.tv_nsec / VOLC_NANOS_IN_A_MILLISECONDS_SECOND;
}

uint32_t volc_timestamp_format_with_ms_and_timezone(char *p_dest_buffer, uint32_t dest_buffer_len, uint64_t timestamp_milliseconds)
{
#if 1
    uint32_t str_len = 0;
    time_t seconds = timestamp_milliseconds / 1000;
    long long milliseconds = (timestamp_milliseconds % 1000000) / 1000;
    struct tm *timeinfo = localtime(&seconds);

    str_len = snprintf(p_dest_buffer, dest_buffer_len - 1, "%4d-%02d-%02d %02d:%02d:%02d.%03d ", timeinfo->tm_year, timeinfo->tm_mon + 1, timeinfo->tm_mday,
                       timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, (timestamp_milliseconds % 1000LL));
    return str_len;
#endif // 0
}

uint32_t volc_generate_timestamp_str(uint64_t timestamp, char *format_str, char *p_dest_buffer, uint32_t dest_buffer_len, uint32_t *p_formatted_str_len)
{
#if 1
//    volc_debug("%s\n", __FUNCTION__);
//    printf("%s time:%d %s \n", __FUNCTION__, timestamp, p_dest_buffer);

    time_t timestamp_seconds;
    uint32_t formatted_str_len = 0;
    uint64_t timestamp_millisecond = 0;

    if (NULL == p_dest_buffer || NULL == p_formatted_str_len) {
        return VOLC_FAILED;
    }

    if (strnlen(format_str, VOLC_MAX_TIMESTAMP_FORMAT_STR_LEN + 1) > VOLC_MAX_TIMESTAMP_FORMAT_STR_LEN) {
        return VOLC_FAILED;
    }

    formatted_str_len = 0;
    *p_formatted_str_len = 0;

    // TODO: maybe we should use a universal method to replace it
    if (0 == strncmp(format_str, "%Y-%m-%d %H:%M:%S.xxx ", VOLC_MAX_TIMESTAMP_FORMAT_STR_LEN)) {
        timestamp_millisecond = timestamp / VOLC_HUNDREDS_OF_NANOS_IN_A_MILLISECOND;
        formatted_str_len = volc_timestamp_format_with_ms_and_timezone(p_dest_buffer, dest_buffer_len, timestamp_millisecond);
    } else {
        timestamp_seconds = timestamp / VOLC_HUNDREDS_OF_NANOS_IN_A_SECOND;
        formatted_str_len = (uint32_t) strftime(p_dest_buffer, dest_buffer_len, format_str, gmtime(&timestamp_seconds));
    }

    if (0 == formatted_str_len) {
        return VOLC_FAILED;
    }

    if (formatted_str_len >= dest_buffer_len) {
        formatted_str_len = dest_buffer_len - 1;
    }
    p_dest_buffer[formatted_str_len] = '\0';
    *p_formatted_str_len = formatted_str_len;
#endif // 0
    return VOLC_SUCCESS;
}

uint32_t volc_get_time_str(char *time_str, uint32_t maxsize)
{
#if 1
    uint32_t ret = VOLC_STATUS_SUCCESS;
    time_t timestamp_seconds = volc_get_time_ms() / VOLC_MILLISECONDS_IN_A_SECOND;
    VOLC_CHK(time_str != NULL, VOLC_STATUS_NULL_ARG);
    strftime(time_str, maxsize, "%Y_%m_%d_%H%M%S", localtime(&timestamp_seconds));
err_out_label:
    return ret;
#endif // 0
}
