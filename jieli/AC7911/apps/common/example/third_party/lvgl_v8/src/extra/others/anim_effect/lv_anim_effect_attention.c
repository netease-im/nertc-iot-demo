#include "lv_anim_effect_attention.h"
#if LV_USE_ANIM_EFFECT

static void custom_scale_y(lv_anim_t *anim, int32_t val)
{
    lv_obj_t *obj = anim->user_data;
    lv_anim_effect_scale_y(obj, val);
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
static void custom_rotate_top_center(lv_anim_t *anim, int32_t val)
{
    lv_obj_t *obj = anim->user_data;
    lv_anim_effect_rotate_top_center(obj, val);
}

static void custom_rotate_center(lv_anim_t *anim, int32_t val)
{
    lv_obj_t *obj = anim->user_data;
    lv_anim_effect_rotate_center(obj, val);
}

void lv_anim_effect_attention_flash(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    lv_anim_t anim_0;
    lv_anim_init(&anim_0);
    lv_anim_set_var(&anim_0, obj);
    lv_anim_set_values(&anim_0, LV_OPA_100, LV_OPA_0);
    lv_anim_set_early_apply(&anim_0, false);
    lv_anim_set_exec_cb(&anim_0, (lv_anim_exec_xcb_t)lv_anim_effect_set_opa);
    lv_anim_set_path_cb(&anim_0, lv_anim_path_ease_in_out);
    lv_anim_set_delay(&anim_0, anim_args->delay);
    lv_anim_set_time(&anim_0, lv_anim_effect_get_time(anim_args->duration, 25));
    lv_anim_set_playback_time(&anim_0, lv_anim_effect_get_time(anim_args->duration, 25));
    lv_anim_set_playback_delay(&anim_0, 0);
    lv_anim_set_repeat_count(&anim_0, 2);
    lv_anim_set_repeat_delay(&anim_0, 0);
    lv_anim_start(&anim_0);
}

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

void lv_anim_effect_attention_bounce(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{

    // y轴变化
    int32_t steps[] = {20, 43, 53, 70, 80, 90, 100};
    int32_t values[] = {0, -30, 0, -15, 0, -4, 0};
    for (int32_t i = 0; i < 6; i++) {
        int32_t start = lv_anim_effect_get_time(anim_args->duration, steps[i]);
        int32_t end = lv_anim_effect_get_time(anim_args->duration, steps[i + 1]);
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_early_apply(&anim, false);
        lv_anim_set_values(&anim, values[i], values[i + 1]);
        lv_anim_set_delay(&anim, anim_args->delay + start);
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)custom_translate_y);
        lv_anim_set_path_cb(&anim, lv_anim_path_custom_bezier);
        lv_anim_set_user_data(&anim, obj);
        lv_anim_set_time(&anim, end - start);
        lv_anim_start(&anim);
    }

    // 高度缩放
    int32_t steps1[] = {0, 43, 70, 80, 90, 100};
    int32_t values1[] = {100, 110, 105, 95, 102, 100};
    for (int32_t i = 0; i < 5; i++) {
        int32_t start = lv_anim_effect_get_time(anim_args->duration, steps1[i]);
        int32_t end = lv_anim_effect_get_time(anim_args->duration, steps1[i + 1]);
        int32_t v_start = values1[i];
        int32_t v_end = values1[i + 1];
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_early_apply(&anim, false);
        lv_anim_set_values(&anim, v_start, v_end);
        lv_anim_set_delay(&anim, anim_args->delay + start);
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)custom_scale_y);
        lv_anim_set_path_cb(&anim, lv_anim_path_custom_bezier);
        lv_anim_set_user_data(&anim, obj);
        lv_anim_set_time(&anim, end - start);
        lv_anim_start(&anim);
    }
}

void lv_anim_effect_attention_head_beat(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 高度缩放
    int32_t steps1[] = {0, 14, 28, 42, 70};
    int32_t values1[] = {100, 130, 100, 130, 100};
    for (int32_t i = 0; i < 4; i++) {
        int32_t start = lv_anim_effect_get_time(anim_args->duration, steps1[i]);
        int32_t end = lv_anim_effect_get_time(anim_args->duration, steps1[i + 1]);

        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_early_apply(&anim, false);
        lv_anim_set_values(&anim, values1[i], values1[i + 1]);
        lv_anim_set_delay(&anim, anim_args->delay + start);
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)custom_scale);
        lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
        lv_anim_set_user_data(&anim, obj);
        lv_anim_set_time(&anim, end - start);
        lv_anim_start(&anim);
    }
}
void lv_anim_effect_attention_head_shake(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // x轴变化
    int32_t steps[] = {0, 7, 19, 32, 43, 50};
    int32_t values[] = {0, -6, 5, -3, 2, 0};
    for (int32_t i = 0; i < 5; i++) {
        int32_t start = lv_anim_effect_get_time(anim_args->duration, steps[i]);
        int32_t end = lv_anim_effect_get_time(anim_args->duration, steps[i + 1]);
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_early_apply(&anim, false);
        lv_anim_set_values(&anim, values[i], values[i + 1]);
        lv_anim_set_delay(&anim, anim_args->delay + start);
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)custom_translate_x);
        lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
        lv_anim_set_user_data(&anim, obj);
        lv_anim_set_time(&anim, end - start);
        lv_anim_start(&anim);
    }
}

void lv_anim_effect_attention_pulse(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 缩放
    lv_anim_t anim_0;
    lv_anim_init(&anim_0);
    lv_anim_set_var(&anim_0, obj);
    lv_anim_set_values(&anim_0, 100, 105);
    lv_anim_set_early_apply(&anim_0, false);
    lv_anim_set_exec_cb(&anim_0, (lv_anim_exec_xcb_t)lv_anim_effect_scale);
    lv_anim_set_path_cb(&anim_0, lv_anim_path_ease_in_out);
    lv_anim_set_delay(&anim_0, anim_args->delay);
    lv_anim_set_time(&anim_0, lv_anim_effect_get_time(anim_args->duration, 50));
    lv_anim_set_playback_time(&anim_0, lv_anim_effect_get_time(anim_args->duration, 50));
    lv_anim_set_playback_delay(&anim_0, 0);
    lv_anim_start(&anim_0);
}
void lv_anim_effect_attention_shake_x(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // x轴变化
    int32_t steps[] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int32_t values[] = {0, -10, 10, -10, 10, -10, 10, -10, 10, -10, 0};

    for (int32_t i = 0; i < 10; i++) {
        int32_t start = lv_anim_effect_get_time(anim_args->duration, steps[i]);
        int32_t end = lv_anim_effect_get_time(anim_args->duration, steps[i + 1]);
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_early_apply(&anim, false);
        lv_anim_set_values(&anim, values[i], values[i + 1]);
        lv_anim_set_delay(&anim, anim_args->delay + start);
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)custom_translate_x);
        lv_anim_set_path_cb(&anim, lv_anim_path_linear);
        lv_anim_set_user_data(&anim, obj);
        lv_anim_set_time(&anim, end - start);
        lv_anim_start(&anim);
    }
}
void lv_anim_effect_attention_shake_y(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // x轴变化
    int32_t steps[] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int32_t values[] = {0, -10, 10, -10, 10, -10, 10, -10, 10, -10, 0};
    for (int32_t i = 0; i < 11; i++) {
        int32_t start = lv_anim_effect_get_time(anim_args->duration, steps[i]);
        int32_t end = lv_anim_effect_get_time(anim_args->duration, steps[i + 1]);
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_early_apply(&anim, false);
        lv_anim_set_values(&anim, values[i], values[i + 1]);
        lv_anim_set_delay(&anim, anim_args->delay + start);
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)custom_translate_y);
        lv_anim_set_path_cb(&anim, lv_anim_path_linear);
        lv_anim_set_user_data(&anim, obj);
        lv_anim_set_time(&anim, end - start);
        lv_anim_start(&anim);
    }
}
void lv_anim_effect_attention_swing(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{

    // 旋转
    int32_t steps[] = {0, 20, 40, 60, 80, 100};
    int32_t values[] = {0, 15, -10, 5, -5, 0};
    for (int32_t i = 0; i < 5; i++) {
        int32_t start = lv_anim_effect_get_time(anim_args->duration, steps[i]);
        int32_t end = lv_anim_effect_get_time(anim_args->duration, steps[i + 1]);
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_early_apply(&anim, false);
        lv_anim_set_values(&anim, values[i], values[i + 1]);
        lv_anim_set_delay(&anim, anim_args->delay + start);
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)custom_rotate_top_center);
        lv_anim_set_path_cb(&anim, lv_anim_path_linear);
        lv_anim_set_user_data(&anim, obj);
        lv_anim_set_time(&anim, end - start);
        lv_anim_start(&anim);
    }
}
void lv_anim_effect_attention_tada(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 旋转
    int32_t steps[] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 85, 100};
    int32_t values[] = {0, -3, -3, 3, -3, 3, -3, 3, -3, 0, 0};
    for (int32_t i = 0; i < 10; i++) {
        int32_t start = lv_anim_effect_get_time(anim_args->duration, steps[i]);
        int32_t end = lv_anim_effect_get_time(anim_args->duration, steps[i + 1]);
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_early_apply(&anim, false);
        lv_anim_set_values(&anim, values[i], values[i + 1]);
        lv_anim_set_delay(&anim, anim_args->delay + start);
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)custom_rotate_center);
        lv_anim_set_path_cb(&anim, lv_anim_path_linear);
        lv_anim_set_user_data(&anim, obj);
        lv_anim_set_time(&anim, end - start);
        lv_anim_start(&anim);
    }

    // 缩放
    // int32_t steps_1[] = {0, 10, 20, 30, 90, 100};
    // int32_t values_1[] = {100, 90, 90, 110, 110, 100};
    // for (int32_t i = 0; i < 5; i++)
    // {
    //   int32_t start = lv_anim_effect_get_time(anim_args->duration, steps_1[i]);
    //   int32_t end = lv_anim_effect_get_time(anim_args->duration, steps_1[i + 1]);
    //   lv_anim_t anim;
    //   lv_anim_init(&anim);
    //   lv_anim_set_early_apply(&anim, false);
    //   lv_anim_set_values(&anim, values_1[i], values_1[i + 1]);
    //   lv_anim_set_delay(&anim, anim_args->delay + start);
    //   lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)custom_scale);
    //   lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    //   lv_anim_set_user_data(&anim, obj);
    //   lv_anim_set_time(&anim, end - start);
    //   lv_anim_start(&anim);
    // }
}
void lv_anim_effect_attention_wobble(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // x轴平移
    int32_t w = lv_obj_get_style_width(obj, 0);
    int32_t steps[] = {0, 15, 30, 45, 60, 75, 100};
    int32_t values[] = {0, -w * 0.25, w * 0.2, -w * 0.15, w * 0.1, -w * 0.05, 0};
    for (int32_t i = 0; i < 6; i++) {
        int32_t start = lv_anim_effect_get_time(anim_args->duration, steps[i]);
        int32_t end = lv_anim_effect_get_time(anim_args->duration, steps[i + 1]);
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_early_apply(&anim, false);
        lv_anim_set_values(&anim, values[i], values[i + 1]);
        lv_anim_set_delay(&anim, anim_args->delay + start);
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)custom_translate_x);
        lv_anim_set_path_cb(&anim, lv_anim_path_linear);
        lv_anim_set_user_data(&anim, obj);
        lv_anim_set_time(&anim, end - start);
        lv_anim_start(&anim);
    }

    // 旋转
    int32_t steps_1[] = {0, 15, 30, 45, 60, 75, 100};
    int32_t values_1[] = {0, -5, 3, -3, 2, -1, 0};
    for (int32_t i = 0; i < 6; i++) {
        int32_t start = lv_anim_effect_get_time(anim_args->duration, steps_1[i]);
        int32_t end = lv_anim_effect_get_time(anim_args->duration, steps_1[i + 1]);

        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_early_apply(&anim, false);
        lv_anim_set_values(&anim, values_1[i], values_1[i + 1]);
        lv_anim_set_delay(&anim, anim_args->delay + start);
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)custom_rotate_center);
        lv_anim_set_path_cb(&anim, lv_anim_path_linear);
        lv_anim_set_user_data(&anim, obj);
        lv_anim_set_time(&anim, end - start);
        lv_anim_start(&anim);
    }
}

#endif /*LV_USE_ANIM_EFFECT*/
