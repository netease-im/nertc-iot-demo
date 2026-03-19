#include "lv_anim_effect_bounce.h"
#if LV_USE_ANIM_EFFECT
static int32_t lv_anim_path_custom_bezier(const lv_anim_t *a)
{
    /*Calculate the current step*/
    uint32_t t = lv_map(a->act_time, 0, a->time, 0, LV_BEZIER_VAL_MAX);
    int32_t step = lv_bezier3(t, 220, 325, 364, LV_BEZIER_VAL_MAX);

    int32_t new_value;
    new_value = step * (a->end_value - a->start_value);
    new_value = new_value >> LV_BEZIER_VAL_SHIFT;
    new_value += a->start_value;
    return new_value;
}
// 透明
static void set_opa(lv_obj_t *obj, int32_t delay, int32_t time, bool in)
{
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_set_var(&anim, obj);
    lv_anim_effect_set_opa(obj, in ? LV_OPA_0 : LV_OPA_100);
    lv_anim_set_values(&anim, in ? LV_OPA_0 : LV_OPA_100, in ? LV_OPA_100 : LV_OPA_0);
    lv_anim_set_delay(&anim, delay);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_anim_effect_set_opa);
    lv_anim_set_path_cb(&anim, in ? lv_anim_path_custom_bezier : lv_anim_path_linear);
    lv_anim_set_time(&anim, time);
    lv_anim_start(&anim);
}

static void custom_scale(lv_anim_t *anim, int32_t val)
{
    lv_obj_t *obj = anim->user_data;
    lv_anim_effect_scale(obj, val);
}

static void custom_translate_x(lv_anim_t *anim, int32_t val)
{
    lv_obj_t *obj = anim->user_data;
    lv_anim_effect_translate_x(obj, val);
}

static void custom_translate_y(lv_anim_t *anim, int32_t val)
{
    lv_obj_t *obj = anim->user_data;
    lv_anim_effect_translate_y(obj, val);
}

static void set_mutil_scale(lv_obj_t *obj, lv_anim_effect_args_t *anim_args, int32_t *steps, int32_t *values, int cnt, bool in)
{
    for (int32_t i = 0; i < cnt - 1; i++) {
        int32_t start = lv_anim_effect_get_time(anim_args->duration, steps[i]);
        int32_t end = lv_anim_effect_get_time(anim_args->duration, steps[i + 1]);
        int32_t v_start = values[i];
        int32_t v_end = values[i + 1];
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_early_apply(&anim, false);
        // lv_anim_set_var(&anim, obj);
        lv_anim_set_values(&anim, v_start, v_end);
        lv_anim_set_delay(&anim, anim_args->delay + start);
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)custom_scale);
        lv_anim_set_path_cb(&anim, in ? lv_anim_path_custom_bezier : lv_anim_path_linear);
        lv_anim_set_user_data(&anim, obj);
        lv_anim_set_time(&anim, end - start);
        lv_anim_start(&anim);
    }
}

static void set_mutil_tranlate_x(lv_obj_t *obj, lv_anim_effect_args_t *anim_args, int32_t *steps, int32_t *values, int cnt, bool in)
{
    for (int32_t i = 0; i < cnt - 1; i++) {
        int32_t start = lv_anim_effect_get_time(anim_args->duration, steps[i]);
        int32_t end = lv_anim_effect_get_time(anim_args->duration, steps[i + 1]);
        int32_t v_start = values[i];
        int32_t v_end = values[i + 1];
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_early_apply(&anim, false);
        // lv_anim_set_var(&anim, obj);
        lv_anim_set_values(&anim, v_start, v_end);
        lv_anim_set_delay(&anim, anim_args->delay + start);
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)custom_translate_x);
        lv_anim_set_path_cb(&anim, in ? lv_anim_path_custom_bezier : lv_anim_path_linear);

        lv_anim_set_user_data(&anim, obj);
        lv_anim_set_time(&anim, end - start);
        lv_anim_start(&anim);
    }
}

static void set_mutil_tranlate_y(lv_obj_t *obj, lv_anim_effect_args_t *anim_args, int32_t *steps, int32_t *values, int cnt, bool in)
{
    for (int32_t i = 0; i < cnt - 1; i++) {
        int32_t start = lv_anim_effect_get_time(anim_args->duration, steps[i]);
        int32_t end = lv_anim_effect_get_time(anim_args->duration, steps[i + 1]);
        int32_t v_start = values[i];
        int32_t v_end = values[i + 1];
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_early_apply(&anim, false);
        // lv_anim_set_var(&anim, obj);
        lv_anim_set_values(&anim, v_start, v_end);
        lv_anim_set_delay(&anim, anim_args->delay + start);
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)custom_translate_y);
        lv_anim_set_path_cb(&anim, in ? lv_anim_path_custom_bezier : lv_anim_path_linear);
        lv_anim_set_user_data(&anim, obj);
        lv_anim_set_time(&anim, end - start);
        lv_anim_start(&anim);
    }
}

void lv_anim_effect_bounce_in(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{

    int32_t steps[] = {0, 20, 40, 60, 80, 100};
    int32_t values[] = {30, 110, 90, 103, 97, 100};
    set_mutil_scale(obj, anim_args, steps, values, 6, true);
    // 透明度
    int32_t t_60 = lv_anim_effect_get_time(anim_args->duration, 60);
    lv_anim_effect_set_opa(obj, LV_OPA_0);
    set_opa(obj, anim_args->delay, t_60, true);
}

void lv_anim_effect_bounce_in_down(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // y轴变化
    int32_t translate = lv_obj_get_style_y(obj, 0);
    translate = translate << 2;
    int32_t steps[] = {0, 60, 75, 90, 100};
    int32_t values[] = {-translate, 25, -10, 5, 0};
    set_mutil_tranlate_y(obj, anim_args, steps, values, 5, true);
    // 缩放
    int32_t steps_1[] = {0, 60, 75, 90, 100};
    int32_t values_1[] = {300, 90, 95, 98, 100};
    set_mutil_scale(obj, anim_args, steps_1, values_1, 5, true);
    // 透明度
    int32_t t_60 = lv_anim_effect_get_time(anim_args->duration, 60);
    set_opa(obj, anim_args->delay, t_60, true);
}
void lv_anim_effect_bounce_in_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // x轴变化
    int32_t translate = lv_obj_get_style_x(obj, 0);
    translate = translate << 2;
    int32_t steps[5] = {0, 60, 75, 90, 100};
    int32_t values[5] = {-translate, 25, -10, 5, 0};
    set_mutil_tranlate_x(obj, anim_args, steps, values, 4, true);
    // 缩放
    int32_t steps_1[] = {0, 60, 75, 100};
    int32_t values_1[] = {300, 100, 98, 100};
    set_mutil_scale(obj, anim_args, steps_1, values_1, 4, true);
    // 透明度
    int32_t t_60 = lv_anim_effect_get_time(anim_args->duration, 60);
    set_opa(obj, anim_args->delay, t_60, true);
}
void lv_anim_effect_bounce_in_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // x轴变化
    lv_disp_t *d = lv_obj_get_disp(obj);
    int32_t translate = lv_disp_get_hor_res(d) - lv_obj_get_style_x(obj, 0);
    translate = translate << 2;
    int32_t steps[5] = {0, 60, 75, 90, 100};
    int32_t values[5] = {translate, -25, 10, -5, 0};
    set_mutil_tranlate_x(obj, anim_args, steps, values, 5, true);
    // 缩放

    int32_t steps_1[] = {0, 60, 75, 100};
    int32_t values_1[] = {300, 100, 98, 100};
    set_mutil_scale(obj, anim_args, steps_1, values_1, 4, true);
    // 透明度
    int32_t t_60 = lv_anim_effect_get_time(anim_args->duration, 60);
    set_opa(obj, anim_args->delay, t_60, true);
}
void lv_anim_effect_bounce_in_up(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // y轴变化
    lv_disp_t *d = lv_obj_get_disp(obj);
    int32_t translate = lv_disp_get_ver_res(d) - lv_obj_get_style_y(obj, 0);
    translate = translate << 2;
    int32_t steps[5] = {0, 60, 75, 90, 100};
    int32_t values[5] = {translate, -20, 10, -5, 0};
    set_mutil_tranlate_y(obj, anim_args, steps, values, 5, true);
    // 缩放
    int32_t steps_1[] = {0, 60, 75, 90, 100};
    int32_t values_1[] = {500, 90, 95, 98, 100};
    set_mutil_scale(obj, anim_args, steps_1, values_1, 5, true);
    // 透明度
    int32_t t_60 = lv_anim_effect_get_time(anim_args->duration, 60);
    set_opa(obj, anim_args->delay, t_60, true);
}

void lv_anim_effect_bounce_out(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 缩放
    int32_t steps[] = {0, 20, 55, 100};
    int32_t values[] = {100, 90, 110, 30};
    set_mutil_scale(obj, anim_args, steps, values, 4, false);
    // 透明度
    int32_t t_55 = lv_anim_effect_get_time(anim_args->duration, 55);
    set_opa(obj, anim_args->delay + t_55, anim_args->duration - t_55, false);
}
void lv_anim_effect_bounce_out_down(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // y轴变化
    int32_t translate = lv_obj_get_style_y(obj, 0);
    translate = translate << 2;
    int32_t steps[] = {0, 20, 45, 100};
    int32_t values[] = {0, 10, -20, translate};
    set_mutil_tranlate_y(obj, anim_args, steps, values, 5, false);
    // 缩放
    int32_t steps_1[] = {0, 20, 45, 100};
    int32_t values_1[] = {100, 98, 90, 300};
    set_mutil_scale(obj, anim_args, steps_1, values_1, 4, false);

    // 透明度
    int32_t t_45 = lv_anim_effect_get_time(anim_args->duration, 45);
    set_opa(obj, anim_args->delay + t_45, anim_args->duration - t_45, false);
}
void lv_anim_effect_bounce_out_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // x轴变化
    int32_t translate = lv_obj_get_style_x(obj, 0);
    translate = translate << 2;
    int32_t steps[] = {0, 20, 100};
    int32_t values[] = {0, 20, -translate};
    set_mutil_tranlate_x(obj, anim_args, steps, values, 3, false);
    // 缩放
    int32_t steps_1[] = {0, 20, 100};
    int32_t values_1[] = {100, 90, 200};
    set_mutil_scale(obj, anim_args, steps_1, values_1, 3, false);
    // 透明度
    int32_t t_20 = lv_anim_effect_get_time(anim_args->duration, 20);
    set_opa(obj, anim_args->delay + t_20, anim_args->duration - t_20, false);
}
void lv_anim_effect_bounce_out_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{

    // x轴变化
    lv_disp_t *d = lv_obj_get_disp(obj);
    int32_t translate = lv_disp_get_hor_res(d) - lv_obj_get_style_x(obj, 0);
    translate = translate << 2;
    int32_t steps[] = {0, 10, 100};
    int32_t values[] = {0, -20, translate};
    set_mutil_tranlate_x(obj, anim_args, steps, values, 3, false);
    // 缩放

    int32_t steps_1[] = {0, 20, 100};
    int32_t values_1[] = {100, 90, 200};
    set_mutil_scale(obj, anim_args, steps_1, values_1, 3, false);

    // 透明度
    int32_t t_20 = lv_anim_effect_get_time(anim_args->duration, 20);
    set_opa(obj, anim_args->delay + t_20, anim_args->duration - t_20, false);
}

void lv_anim_effect_bounce_out_up(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{

    // y轴变化
    lv_disp_t *d = lv_obj_get_disp(obj);
    int32_t translate = lv_disp_get_ver_res(d) - lv_obj_get_style_y(obj, 0);
    translate = translate << 2;
    int32_t steps[] = {0, 20, 45, 100};
    int32_t values[] = {0, -10, 20, -translate};
    set_mutil_tranlate_y(obj, anim_args, steps, values, 4, false);

    // 缩放
    int32_t steps_1[] = {0, 20, 45, 100};
    int32_t values_1[] = {100, 98, 90, 300};
    set_mutil_scale(obj, anim_args, steps_1, values_1, 4, false);

    // 透明度
    int32_t t_45 = lv_anim_effect_get_time(anim_args->duration, 45);
    set_opa(obj, anim_args->delay + t_45, anim_args->duration - t_45, false);
}

#endif /*LV_USE_ANIM_EFFECT*/
