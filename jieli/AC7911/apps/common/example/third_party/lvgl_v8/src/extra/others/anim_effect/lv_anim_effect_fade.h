#ifndef LV_ANIM_EFFECT_FADE_H
#define LV_ANIM_EFFECT_FADE_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "lv_anim_effect_global.h"
#if LV_USE_ANIM_EFFECT

void lv_anim_effect_fade_in(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_in_bottom_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_in_bottom_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_in_down(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_in_down_big(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_in_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_in_left_big(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_in_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_in_right_big(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_in_top_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_in_top_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_in_up(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_in_up_big(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);

void lv_anim_effect_fade_out(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_out_bottom_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_out_bottom_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_out_down(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_out_down_big(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_out_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_out_left_big(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_out_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_out_right_big(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_out_top_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_out_top_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_out_up(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_fade_out_up_big(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
#endif /*LV_USE_ANIM_EFFECT*/
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif