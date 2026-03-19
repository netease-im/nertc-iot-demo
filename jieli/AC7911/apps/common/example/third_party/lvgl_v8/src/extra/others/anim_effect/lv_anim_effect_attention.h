#ifndef LV_ANIM_EFFECT_ATTENTION_H
#define LV_ANIM_EFFECT_ATTENTION_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "lv_anim_effect_global.h"
#if LV_USE_ANIM_EFFECT

void lv_anim_effect_attention_flash(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_attention_bounce(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_attention_head_beat(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_attention_head_shake(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);

void lv_anim_effect_attention_pulse(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_attention_shake_x(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_attention_shake_y(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_attention_swing(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_attention_swing(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_attention_tada(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_attention_wobble(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);

#endif /*LV_USE_ANIM_EFFECT*/
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif