#ifndef _NERTC_WAKE_UP_H_
#define _NERTC_WAKE_UP_H_

#include "nertc_wakeup_sdk_event.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NERTC_WAKE_UP_API __attribute__((visibility("default")))

/**
 * @brief 创建 wakeup 实例,该方法是整个SDK调用的第一个方法
 * @param cfg 引擎配置
 * @return 引擎实例
 */
NERTC_WAKE_UP_API nertc_wakeup_sdk_t nertc_wakeup_create(const nertc_wakeup_sdk_config_t *cfg);

/**
 * @brief 销毁 wakeup 实例
 * @param engine 通过 nertc_wakeup_create 创建的实例
 */
NERTC_WAKE_UP_API void nertc_wakeup_destory(nertc_wakeup_sdk_t wakeup);

/**
 * @brief 初始化 wakeup 实例
 * @note  创建引擎实例之后调用的第一个方法，仅能被初始化一次
 * @param engine 通过 nertc_wakeup_create 创建且未被初始化的引擎实例
 * @return 方法调用结果：<br>
 *         -   0：成功 <br>
 *         - 非0：失败 <br>
 */
NERTC_WAKE_UP_API int nertc_wakeup_init(nertc_wakeup_sdk_t wakeup, int input_channels, int reference_channels);

NERTC_WAKE_UP_API int nertc_wakeup_detect(nertc_wakeup_sdk_t wakeup);

NERTC_WAKE_UP_API int nertc_wakeup_stop_detect(nertc_wakeup_sdk_t wakeup);

NERTC_WAKE_UP_API int nertc_wakeup_feed(nertc_wakeup_sdk_t wakeup, const int16_t* data, int data_length);

NERTC_WAKE_UP_API int nertc_wakeup_get_feed_size(nertc_wakeup_sdk_t wakeup);

NERTC_WAKE_UP_API int nertc_wakeup_get_feed_chunk_size(nertc_wakeup_sdk_t wakeup);

NERTC_WAKE_UP_API int nertc_wakeup_get_fetch_chunk_size(nertc_wakeup_sdk_t wakeup);

#ifdef __cplusplus
}
#endif
#endif