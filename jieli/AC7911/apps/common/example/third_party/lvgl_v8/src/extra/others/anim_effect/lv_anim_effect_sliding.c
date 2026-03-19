#include "lv_anim_effect_sliding.h"
#if LV_USE_ANIM_EFFECT

static void tranlate_x(lv_obj_t *obj, lv_anim_effect_args_t *anim_args, int32_t start, int32_t end)
{
    // x轴
    lv_anim_t anim;

    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_effect_translate_x(obj, start);
    lv_anim_set_values(&anim, start, end);
    lv_anim_set_delay(&anim, anim_args->delay);
    lv_anim_set_early_apply(&anim, false);

    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_anim_effect_translate_x);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_time(&anim, anim_args->duration);
    lv_anim_start(&anim);
}

static void tranlate_y(lv_obj_t *obj, lv_anim_effect_args_t *anim_args, int32_t start, int32_t end)
{
    // x轴
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_effect_translate_y(obj, start);
    lv_anim_set_values(&anim, start, end);
    lv_anim_set_delay(&anim, anim_args->delay);
    lv_anim_set_early_apply(&anim, false);

    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_anim_effect_translate_y);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_time(&anim, anim_args->duration);
    lv_anim_start(&anim);
}

static void set_end_opa(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // x轴
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_values(&anim, LV_OPA_10, LV_OPA_0);
    lv_anim_set_delay(&anim, anim_args->delay + anim_args->duration);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_anim_effect_set_opa);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_set_time(&anim, LV_DISP_DEF_REFR_PERIOD * 2);
    lv_anim_start(&anim);
}

void lv_anim_effect_sliding_in_down(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    int32_t translate = lv_obj_get_style_height(obj, 0);
    tranlate_y(obj, anim_args, -translate, 0);
    lv_anim_effect_set_opa(obj, LV_OPA_100);
}
void lv_anim_effect_sliding_in_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    int32_t translate = lv_obj_get_style_width(obj, 0);
    tranlate_x(obj, anim_args, -translate, 0);
    lv_anim_effect_set_opa(obj, LV_OPA_100);
}
void lv_anim_effect_sliding_in_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    int32_t translate = lv_obj_get_style_width(obj, 0);
    tranlate_x(obj, anim_args, translate, 0);
    lv_anim_effect_set_opa(obj, LV_OPA_100);
}
void lv_anim_effect_sliding_in_up(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    int32_t translate = lv_obj_get_style_height(obj, 0);
    tranlate_y(obj, anim_args, translate, 0);
    lv_anim_effect_set_opa(obj, LV_OPA_100);
}

void lv_anim_effect_sliding_out_down(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    int32_t translate = lv_obj_get_style_height(obj, 0);
    tranlate_y(obj, anim_args, 0, translate);
    set_end_opa(obj, anim_args);
}
void lv_anim_effect_sliding_out_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    int32_t translate = lv_obj_get_style_width(obj, 0);
    tranlate_x(obj, anim_args, 0, -translate);
    set_end_opa(obj, anim_args);
}
void lv_anim_effect_sliding_out_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    int32_t translate = lv_obj_get_style_width(obj, 0);
    tranlate_x(obj, anim_args, 0, translate);
    set_end_opa(obj, anim_args);
}
void lv_anim_effect_sliding_out_up(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    int32_t translate = lv_obj_get_style_height(obj, 0);
    tranlate_y(obj, anim_args, 0, -translate);
    set_end_opa(obj, anim_args);
}

#endif /*LV_USE_ANIM_EFFECT*/
