#include "lv_anim_effect_rotate.h"
#if LV_USE_ANIM_EFFECT

static void set_rotate(lv_obj_t *obj, lv_anim_effect_args_t *anim_args, lv_anim_exec_xcb_t cb, int32_t start, int32_t end)
{
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_values(&anim, start, end);
    lv_anim_set_delay(&anim, anim_args->delay);
    lv_anim_set_exec_cb(&anim, cb);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_time(&anim, anim_args->duration);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_start(&anim);
}

// 透明
static void set_opa(lv_obj_t *obj, lv_anim_effect_args_t *anim_args, bool in)
{
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_effect_set_opa(obj, in ? LV_OPA_0 : LV_OPA_100);
    lv_anim_set_values(&anim, in ? LV_OPA_0 : LV_OPA_100, in ? LV_OPA_100 : LV_OPA_0);
    lv_anim_set_delay(&anim, anim_args->delay);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_anim_effect_set_opa);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_time(&anim, anim_args->duration);
    lv_anim_start(&anim);
}

void lv_anim_effect_rotate_in_down_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明度
    set_opa(obj, anim_args, true);
    // 旋转
    set_rotate(obj, anim_args, (lv_anim_exec_xcb_t)lv_anim_effect_rotate_bottom_left, -45, 0);
}
void lv_anim_effect_rotate_in_down_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明度
    set_opa(obj, anim_args, true);
    // 旋转
    set_rotate(obj, anim_args, (lv_anim_exec_xcb_t)lv_anim_effect_rotate_bottom_right, 45, 0);
}
void lv_anim_effect_rotate_in_up_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明度
    set_opa(obj, anim_args, true);
    // 旋转
    set_rotate(obj, anim_args, (lv_anim_exec_xcb_t)lv_anim_effect_rotate_bottom_left, 45, 0);
}
void lv_anim_effect_rotate_in_up_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明度
    set_opa(obj, anim_args, true);
    // 旋转
    set_rotate(obj, anim_args, (lv_anim_exec_xcb_t)lv_anim_effect_rotate_bottom_right, -90, 0);
}
void lv_anim_effect_rotate_in(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明度
    set_opa(obj, anim_args, true);
    // 旋转
    set_rotate(obj, anim_args, (lv_anim_exec_xcb_t)lv_anim_effect_rotate_center, -200, 0);
}

void lv_anim_effect_rotate_out_down_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明度
    set_opa(obj, anim_args, false);
    // 旋转
    set_rotate(obj, anim_args, (lv_anim_exec_xcb_t)lv_anim_effect_rotate_bottom_left, 0, 45);
}
void lv_anim_effect_rotate_out_down_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明度
    set_opa(obj, anim_args, false);
    // 旋转
    set_rotate(obj, anim_args, (lv_anim_exec_xcb_t)lv_anim_effect_rotate_bottom_right, 0, -45);
}
void lv_anim_effect_rotate_out_up_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明度
    set_opa(obj, anim_args, false);
    // 旋转
    set_rotate(obj, anim_args, (lv_anim_exec_xcb_t)lv_anim_effect_rotate_bottom_left, 0, -45);
}
void lv_anim_effect_rotate_out_up_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明度
    set_opa(obj, anim_args, false);
    // 旋转
    set_rotate(obj, anim_args, (lv_anim_exec_xcb_t)lv_anim_effect_rotate_bottom_right, 0, 90);
}
void lv_anim_effect_rotate_out(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明度
    set_opa(obj, anim_args, false);
    // 旋转
    set_rotate(obj, anim_args, (lv_anim_exec_xcb_t)lv_anim_effect_rotate_center, 0, 200);
}

#endif /*LV_USE_ANIM_EFFECT*/
