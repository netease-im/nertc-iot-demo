#ifndef LV_ANIM_EFFECT_H
#define LV_ANIM_EFFECT_H

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * 试用效果动画需要开启 配置lv_conf.h：
 *      #define CONFIG_LV_COLOR_SCREEN_TRANSP = 1
 * 否则，旋转和缩放都会显示异常；
 */
#if LV_USE_ANIM_EFFECT
typedef struct _lv_anim_effect_args_t {
    int32_t duration; // 效果时长
    int32_t delay;    // 延时
    // uint16_t repeat_cnt;   // 0:一直重复
    // uint32_t repeat_delay; // 重复延时时间 （重复间隔）
    // uint8_t early_apply;   // 动画是否加载的时候就执行
} lv_anim_effect_args_t;

#include "lv_anim_effect_global.h"
#include "lv_anim_effect_attention.h"
#include "lv_anim_effect_bounce.h"
#include "lv_anim_effect_fade.h"
#include "lv_anim_effect_back.h"
#include "lv_anim_effect_sliding.h"
#include "lv_anim_effect_rotate.h"
#include "lv_anim_effect_zooming.h"
#include "lv_anim_effect_specials.h"
#endif /*LV_USE_ANIM_EFFECT*/
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_ANIM_EFFECT_H*/
