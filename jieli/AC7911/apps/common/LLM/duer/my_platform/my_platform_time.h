#ifndef _MY_PLATFORM_TIME_H_
#define _MY_PLATFORM_TIME_H_




#include "system/includes.h"
#include "sys_time.h"
#include <time.h>
#include "my_platform_common.h"

#define UTC_BASE_YEAR 		1970
#define MONTH_PER_YEAR 		12
#define DAY_PER_YEAR 		365
#define SEC_PER_DAY 		86400
#define SEC_PER_HOUR 		3600
#define SEC_PER_MIN 		60
#define IOCTL_GET_SYS_TIME              19
extern void *dev_open(const char *name, void *arg);
extern int dev_close(void *_device);
extern int dev_ioctl(void *_device, int cmd, u32 arg);
extern int ntp_client_get_time(const char *host);



extern void my_get_sys_time(struct sys_time *time);
extern void my_utc_sec_2_mytime(u32 utc_sec, struct sys_time *mytime);
extern u32 my_mytime_2_utc_sec(struct sys_time *mytime);
extern void my_get_timestamp(const struct sys_time *t, char *buffer, size_t size);
extern size_t my_strftime_2(char *ptr, size_t maxsize, const char *format, const struct tm *timeptr);

#endif
