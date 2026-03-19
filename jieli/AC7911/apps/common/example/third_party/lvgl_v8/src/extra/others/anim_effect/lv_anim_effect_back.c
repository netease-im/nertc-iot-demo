#include "lv_anim_effect_back.h"
#if LV_USE_ANIM_EFFECT

static void tranlate_x(lv_obj_t *obj, int32_t delay, int32_t time, int32_t start, int32_t end)
{
    // x轴
    lv_anim_t anim;

    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_early_apply(&anim, false);

    lv_anim_set_values(&anim, start, end);
    lv_anim_set_delay(&anim, delay);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_anim_effect_translate_x);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_time(&anim, time);
    lv_anim_set_early_apply(&anim, false);

    lv_anim_start(&anim);
}

static void tranlate_y(lv_obj_t *obj, int32_t delay, int32_t time, int32_t start, int32_t end)
{
    // x轴
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_set_var(&anim, obj);
    lv_anim_effect_translate_y(obj, start);
    lv_anim_set_values(&anim, start, end);
    lv_anim_set_delay(&anim, delay);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_anim_effect_translate_y);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_time(&anim, time);
    lv_anim_start(&anim);
}

static void set_opa(lv_obj_t *obj, int32_t delay, int32_t time, int32_t start, int32_t end)
{
    // x轴
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_values(&anim, start, end);
    lv_anim_set_delay(&anim, delay);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_anim_effect_set_opa);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_set_time(&anim, time);
    lv_anim_start(&anim);
}

static void set_scale(lv_obj_t *obj, int32_t delay, int32_t time, int32_t start, int32_t end)
{
    // x轴
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_values(&anim, start, end);
    lv_anim_set_delay(&anim, delay);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_anim_effect_scale);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_set_time(&anim, time);
    lv_anim_start(&anim);
}

void lv_anim_effect_back_in_down(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{

    int32_t t_start = lv_anim_effect_get_time(anim_args->duration, 80);
    int32_t time = anim_args->duration - t_start;
    // 缩放
    set_scale(obj, anim_args->delay + t_start, time, 70, 100);
    // 透明
    set_opa(obj, anim_args->delay + t_start, time, LV_OPA_70, LV_OPA_100);
    // y轴
    int32_t translate = lv_obj_get_style_y(obj, 0) + lv_obj_get_style_height(obj, 0);
    tranlate_y(obj, anim_args->delay, t_start, -translate, 0);
}

void lv_anim_effect_back_in_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 缩放
    int32_t t_start = lv_anim_effect_get_time(anim_args->duration, 80);
    int32_t time = anim_args->duration - t_start;
    // 缩放
    set_scale(obj, anim_args->delay + t_start, time, 70, 100);
    // 透明
    set_opa(obj, anim_args->delay + t_start, time, LV_OPA_70, LV_OPA_100);

    // x轴
    int32_t translate = lv_obj_get_style_x(obj, 0) + lv_obj_get_style_width(obj, 0);
    tranlate_x(obj, anim_args->delay, t_start, -translate, 0);
}
void lv_anim_effect_back_in_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 缩放
    int32_t t_start = lv_anim_effect_get_time(anim_args->duration, 80);
    int32_t time = anim_args->duration - t_start;
    // 缩放
    set_scale(obj, anim_args->delay + t_start, time, 70, 100);
    // 透明
    set_opa(obj, anim_args->delay + t_start, time, LV_OPA_70, LV_OPA_100);
    // x轴
    lv_disp_t *d = lv_obj_get_disp(obj);
    int32_t translate = lv_disp_get_hor_res(d) - lv_obj_get_style_x(obj, 0); // 屏幕宽度-控件x轴坐标
    tranlate_x(obj, anim_args->delay, t_start, translate, 0);
}

void lv_anim_effect_back_in_up(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    int32_t t_start = lv_anim_effect_get_time(anim_args->duration, 80);
    int32_t time = anim_args->duration - t_start;
    // 缩放
    set_scale(obj, anim_args->delay + t_start, time, 70, 100);
    // 透明
    set_opa(obj, anim_args->delay + t_start, time, LV_OPA_70, LV_OPA_100);
    // y轴
    lv_disp_t *d = lv_obj_get_disp(obj);
    int32_t translate = lv_disp_get_ver_res(d) - lv_obj_get_style_y(obj, 0); // 屏幕宽度-控件y轴坐标
    tranlate_y(obj, anim_args->delay, t_start, translate, 0);
}

void lv_anim_effect_back_out_down(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    int32_t t_start = lv_anim_effect_get_time(anim_args->duration, 20);
    int32_t time = anim_args->duration - t_start;
    // 缩放
    set_scale(obj, anim_args->delay, t_start, 100, 70);
    // 透明
    set_opa(obj, anim_args->delay, t_start, LV_OPA_100, LV_OPA_70);
    // y轴
    lv_disp_t *d = lv_obj_get_disp(obj);
    int32_t translate = lv_disp_get_ver_res(d) - lv_obj_get_style_y(obj, 0); // 屏幕宽度-控件x轴坐标
    tranlate_y(obj, anim_args->delay + t_start, time, 0, translate);
}
void lv_anim_effect_back_out_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    int32_t t_start = lv_anim_effect_get_time(anim_args->duration, 20);
    int32_t time = anim_args->duration - t_start;
    // 缩放
    set_scale(obj, anim_args->delay, t_start, 100, 70);
    // 透明
    set_opa(obj, anim_args->delay, t_start, LV_OPA_100, LV_OPA_70);
    // x轴
    int32_t translate = lv_obj_get_style_x(obj, 0) + lv_obj_get_style_width(obj, 0); // 屏幕宽度-控件x轴坐标
    tranlate_x(obj, anim_args->delay + t_start, time, 0, -translate);
}
void lv_anim_effect_back_out_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    int32_t t_start = lv_anim_effect_get_time(anim_args->duration, 20);
    int32_t time = anim_args->duration - t_start;
    // 缩放
    set_scale(obj, anim_args->delay, t_start, 100, 70);
    // 透明
    set_opa(obj, anim_args->delay, t_start, LV_OPA_100, LV_OPA_70);
    // x轴
    lv_disp_t *d = lv_obj_get_disp(obj);
    int32_t translate = lv_disp_get_hor_res(d) - lv_obj_get_style_x(obj, 0); // 屏幕宽度-控件x轴坐标
    tranlate_x(obj, anim_args->delay + t_start, time, 0, translate);
}
void lv_anim_effect_back_out_up(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    int32_t t_start = lv_anim_effect_get_time(anim_args->duration, 20);
    int32_t time = anim_args->duration - t_start;
    // 缩放
    set_scale(obj, anim_args->delay, t_start, 100, 70);
    // 透明
    set_opa(obj, anim_args->delay, t_start, LV_OPA_100, LV_OPA_70);
    // y轴
    int32_t translate = lv_obj_get_style_y(obj, 0) + lv_obj_get_style_height(obj, 0); // 屏幕宽度-控件y轴坐标
    tranlate_y(obj, anim_args->delay + t_start, time, 0, -translate);
}

#endif /*LV_USE_ANIM_EFFECT*/
