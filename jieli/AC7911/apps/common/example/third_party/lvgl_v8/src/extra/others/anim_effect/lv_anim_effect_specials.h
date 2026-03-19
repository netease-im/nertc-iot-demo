#ifndef LV_ANIM_EFFECT_SPECIALS_H
#define LV_ANIM_EFFECT_SPECIALS_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "lv_anim_effect_global.h"
#if LV_USE_ANIM_EFFECT

void lv_anim_effect_specials_hinge(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_specials_jack_in_the_box(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_specials_roll_in(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);
void lv_anim_effect_specials_roll_out(lv_obj_t *obj, lv_anim_effect_args_t *anim_args);


#endif /*LV_USE_ANIM_EFFECT*/
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif