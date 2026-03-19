/*
 * Copyright (c) 2024 The VolcEngineRTCLite project authors. All Rights Reserved.
 * @brief VolcEngineRTCLite Interface Lite
 */

#ifndef __HAL_VOLC_COND_H__
#define __HAL_VOLC_COND_H__

#include <stdint.h>

#include "volc_mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#if defined(__BUILDING_BYTE_RTC_SDK__)
#define __byte_rtc_api__ __declspec(dllexport)
#else
#define __byte_rtc_api__ __declspec(dllimport)
#endif
#else
#define __byte_rtc_api__ __attribute__((visibility("default")))
#endif

/**
 * @locale zh
 * @type keytype
 * @brief cond 句柄
 */
typedef void *volc_cond_t;

/**
 * @locale zh
 * @type api
 * @list 方法
 * @brief 条件变量创建
 * @return 方法调用结果：<br>
 *         - 成功: 条件变量句柄 <br>
 *         - 失败: NULL
 */
volc_cond_t volc_cond_create(void);

/**
 * @locale zh
 * @type api
 * @list 方法
 * @brief 唤醒一个等待在条件变量上的线程
 * @param cond 条件变量句柄
 * @return 方法调用结果：<br>
 *         - 成功: VOLC_SUCCESS <br>
 *         - 失败: VOLC_FAILED
 */
uint32_t volc_cond_signal(volc_cond_t cond);

/**
 * @locale zh
 * @type api
 * @list 方法
 * @brief 唤醒所有等待在条件变量上的线程
 * @param cond 条件变量句柄
 * @return 方法调用结果：<br>
 *         - 成功: VOLC_SUCCESS <br>
 *         - 失败: VOLC_FAILED
 */
uint32_t volc_cond_broadcast(volc_cond_t cond);

/**
 * @locale zh
 * @type api
 * @list 方法
 * @brief 在一个条件变量上等待
 * @param cond 条件变量句柄
 * @param mutext 互斥锁句柄
 * @param timeout 超时时间
 * @return 方法调用结果：<br>
 *         - 成功: VOLC_SUCCESS <br>
 *         - 失败: VOLC_FAILED
 */
uint32_t volc_cond_wait(volc_cond_t cond, volc_mutex_t mutex, uint64_t timeout);

/**
 * @locale zh
 * @type api
 * @list 方法
 * @brief 条件变量销毁
 * @param cond 条件变量句柄
 */
void volc_cond_destroy(volc_cond_t cond);

#ifdef __cplusplus
}
#endif
#endif /* __HAL_VOLC_COND_H__ */
