#ifndef LV_ANIM_EFFECT_GLOBAL_H
#define LV_ANIM_EFFECT_GLOBAL_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "../../../lvgl.h"

#if LV_USE_ANIM_EFFECT

// 控件属性处理
void lv_anim_effect_set_opa(lv_obj_t *obj, lv_opa_t opa);
void lv_anim_effect_set_height(lv_obj_t *obj, uint32_t height);
void lv_anim_effect_translate_y(lv_obj_t *obj, int32_t y);
void lv_anim_effect_translate_x(lv_obj_t *obj, int32_t x);
void lv_anim_effect_scale_y(lv_obj_t *obj, int32_t scale);
void lv_anim_effect_scale_x(lv_obj_t *obj, int32_t scale);
void lv_anim_effect_scale(lv_obj_t *obj, int32_t scale);
void lv_anim_effect_scale_center_x(lv_obj_t *obj, int32_t scale);
void lv_anim_effect_scale_center_y(lv_obj_t *obj, int32_t scale);
void lv_anim_effect_rotate_center(lv_obj_t *obj, int32_t deg);
void lv_anim_effect_rotate_top_center(lv_obj_t *obj, int32_t deg);
void lv_anim_effect_rotate_top_left(lv_obj_t *obj, int32_t deg);
void lv_anim_effect_rotate_bottom_right(lv_obj_t *obj, int32_t deg);
void lv_anim_effect_rotate_bottom_left(lv_obj_t *obj, int32_t deg);

// 动画百分比时长转换为毫秒
uint32_t lv_anim_effect_get_time(int32_t duration, int16_t percent);

#endif /*LV_USE_ANIM_EFFECT*/
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_ANIM_EFFECT_GLOBAL_H*/
