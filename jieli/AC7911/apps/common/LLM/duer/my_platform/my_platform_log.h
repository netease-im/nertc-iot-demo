#ifndef _MY_LOG_H_
#define _MY_LOG_H_

#include "system/includes.h"
#define MY_LOG_LEVEL           (4)

#define MY_LOG_ERR_LEVEL       (1)
#define MY_LOG_WARN_LEVEL      (2)
#define MY_LOG_INFO_LEVEL      (3)
#define MY_LOG_DEBUG_LEVEL     (4)

#define __FILENAME__ (strrchr(__FILE__, '/') + 1)

#define MY_LOG_I(format, ...)   do { \
    if (MY_LOG_LEVEL >= MY_LOG_INFO_LEVEL) \
        printf("[MY_INFO] [%s:%d] [%s] | " format "\n", \
               __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
} while (0)

#define MY_LOG_W(format, ...)   do { \
    if (MY_LOG_LEVEL >= MY_LOG_WARN_LEVEL) \
        printf("[MY_WARNING] [%s:%d] [%s] | " format "\n", \
               __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
} while (0)

#define MY_LOG_E(format, ...)   do { \
    if (MY_LOG_LEVEL >= MY_LOG_ERR_LEVEL) \
        printf("[MY_ERROR] [%s:%d] [%s] | " format "\n", \
               __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
} while (0)

#define MY_LOG_D(format, ...)   do { \
    if (MY_LOG_LEVEL >= MY_LOG_DEBUG_LEVEL) \
        printf("[MY_DEBUG] [%s:%d] [%s] | " format "\n", \
               __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
} while (0)

#endif
