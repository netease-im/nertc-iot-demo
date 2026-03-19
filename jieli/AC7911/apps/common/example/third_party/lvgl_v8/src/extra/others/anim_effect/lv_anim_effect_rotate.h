#ifndef LV_ANIM_EFFECT_ROTATE_H
#define LV_ANIM_EFFECT_ROTATE_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "lv_anim_effect_global.h"
#if LV_USE_ANIM_EFFECT

void lv_anim_effect_rotate_in_down_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_rotate_in_down_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_rotate_in_up_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_rotate_in_up_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_rotate_in(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);

void lv_anim_effect_rotate_out_down_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_rotate_out_down_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_rotate_out_up_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_rotate_out_up_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_rotate_out(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);

#endif /*LV_USE_ANIM_EFFECT*/
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif