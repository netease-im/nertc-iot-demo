/**
 * @file lv_os_jieli.h
 *
 */

#ifndef LV_OS_JIELI_H
#define LV_OS_JIELI_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if LV_USE_OS == LV_OS_CUSTOM

#include <stdbool.h>
#include "os/os_api.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    int pid;
} lv_thread_t;

typedef OS_MUTEX lv_mutex_t;

typedef struct {
    OS_SEM sem;
} lv_thread_sync_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_OS == LV_OS_PTHREAD*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PTHREAD_H*/
