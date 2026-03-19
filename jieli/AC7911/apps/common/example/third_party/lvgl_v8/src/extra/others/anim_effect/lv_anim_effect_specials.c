#include "lv_anim_effect_zooming.h"
#if LV_USE_ANIM_EFFECT

static void lv_anim_effect_rotate_top_left_1(lv_obj_t *obj, int32_t val)
{
    lv_anim_effect_rotate_top_left(obj, val);
}

static void handle_jack_in_the_box(lv_obj_t *obj, int32_t val)
{

    uint32_t opa = LV_OPA_100 * val / 100;
    lv_anim_effect_set_opa(obj, opa);

    // 旋转
    uint32_t deg = 90 * val / 100 - 45;
    if (val < 50) {
        deg = 30 - val * 40 / 50;
    } else if (val < 70) {
        deg = (val - 50) * 13 / 20 + 3;
    } else {
        deg = (val - 70) * 3 / 30 - 3;
    }

    uint32_t scale = 90 * val / 100 + 10;
    int32_t w = lv_obj_get_style_width(obj, 0);
    int32_t retW = w * scale / 100;
    int32_t tranX = (retW - w) / 2;

    int32_t h = lv_obj_get_style_height(obj, 0);
    int32_t retH = h * scale / 100;
    int32_t tranY = (retH - h);

    int32_t x = retW / 2;
    int32_t y = retH;
    int16_t sin = lv_trigo_sin(deg);
    int16_t cos = lv_trigo_cos(deg);
    int32_t x1 = x * cos / 32767 - y * sin / 32767;
    int32_t y1 = x * sin / 32767 + y * cos / 32767;
    int32_t dx = x - x1;
    int32_t dy = y - y1;

    lv_obj_set_style_translate_x(obj, -tranX - dx, LV_PART_MAIN);
    lv_obj_set_style_translate_y(obj, -tranY + dy, 0);

    if (lv_obj_check_type(obj, &lv_img_class)) {
        lv_img_set_angle(obj, (uint16_t)deg * 10);
        lv_img_set_zoom(obj, (uint16_t)scale * 2.56); // 256为一倍
    } else {
        lv_obj_set_style_transform_angle(obj, deg * 10, 0);
        lv_obj_set_style_transform_zoom(obj, (uint16_t)scale * 2.56, 0); // 如果图片控件缩放会导致崩溃
    }
}

static void handle_roll_in(lv_obj_t *obj, int32_t val)
{

    int32_t w = lv_obj_get_style_width(obj, 0);
    int32_t h = lv_obj_get_style_height(obj, 0);
    uint32_t opa = LV_OPA_100 * val / 100;
    lv_anim_effect_set_opa(obj, opa);
    uint32_t deg = 120 * val / 100 - 120;
    uint32_t tansX = w * val / 100 - w;
    int32_t x = w / 2;
    int32_t y = h / 2;
    int16_t sin = lv_trigo_sin(deg);
    int16_t cos = lv_trigo_cos(deg);
    int32_t x1 = x * cos / 32767 - y * sin / 32767;
    int32_t y1 = x * sin / 32767 + y * cos / 32767;
    int32_t dx = x - x1;
    int32_t dy = y - y1;
    lv_obj_set_style_translate_x(obj, dx + tansX, 0);
    lv_obj_set_style_translate_y(obj, dy, 0);
    if (lv_obj_check_type(obj, &lv_img_class)) {
        lv_img_set_angle(obj, (uint16_t)deg * 10);
    } else {
        lv_obj_set_style_transform_angle(obj, deg * 10, 0);
    }
}

static void handle_roll_out(lv_obj_t *obj, int32_t val)
{

    int32_t w = lv_obj_get_style_width(obj, 0);
    int32_t h = lv_obj_get_style_height(obj, 0);
    uint32_t opa = LV_OPA_100 - LV_OPA_100 * val / 100;
    lv_anim_effect_set_opa(obj, opa);
    uint32_t deg = 120 * val / 100;
    uint32_t tansX = w * val / 100;
    int32_t x = w / 2;
    int32_t y = h / 2;
    int16_t sin = lv_trigo_sin(deg);
    int16_t cos = lv_trigo_cos(deg);
    int32_t x1 = x * cos / 32767 - y * sin / 32767;
    int32_t y1 = x * sin / 32767 + y * cos / 32767;
    int32_t dx = x - x1;
    int32_t dy = y - y1;
    lv_obj_set_style_translate_x(obj, dx + tansX, 0);
    lv_obj_set_style_translate_y(obj, dy, 0);
    if (lv_obj_check_type(obj, &lv_img_class)) {
        lv_img_set_angle(obj, (uint16_t)deg * 10);
    } else {
        lv_obj_set_style_transform_angle(obj, deg * 10, 0);
    }
}

void lv_anim_effect_specials_hinge(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    uint32_t t_20 = lv_anim_effect_get_time(anim_args->duration, 20);
    // 旋转
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_set_values(&anim, 0, 80);
    lv_anim_set_delay(&anim, anim_args->delay);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_anim_effect_rotate_top_left);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_set_time(&anim, t_20);
    lv_anim_start(&anim);

    // 平移
    lv_anim_set_values(&anim, 0, 700);
    lv_anim_set_delay(&anim, anim_args->delay + anim_args->duration - t_20);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_anim_effect_translate_y);
    lv_anim_set_time(&anim, anim_args->duration);
    lv_anim_start(&anim);

    // 透明
    lv_anim_set_values(&anim, LV_OPA_100, LV_OPA_0);
    lv_anim_set_delay(&anim, anim_args->delay + anim_args->duration - t_20);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_anim_effect_set_opa);
    lv_anim_set_time(&anim, anim_args->duration);
    lv_anim_start(&anim);

    // 抖动旋转
    lv_anim_set_values(&anim, 80, 60);
    lv_anim_set_delay(&anim, anim_args->delay + t_20);
    lv_anim_set_time(&anim, t_20);
    lv_anim_set_playback_time(&anim, t_20);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_anim_effect_rotate_top_left_1);
    lv_anim_set_playback_delay(&anim, 0);
    lv_anim_set_repeat_count(&anim, 1);
    lv_anim_start(&anim);
}

void lv_anim_effect_specials_jack_in_the_box(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{

    // 旋转
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_set_values(&anim, 0, 100);
    lv_anim_set_delay(&anim, anim_args->delay);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)handle_jack_in_the_box);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_time(&anim, anim_args->duration);
    lv_anim_start(&anim);
}

void lv_anim_effect_specials_roll_in(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_set_values(&anim, 0, 100);
    lv_anim_set_delay(&anim, anim_args->delay);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)handle_roll_in);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_time(&anim, anim_args->duration);
    lv_anim_start(&anim);
}
void lv_anim_effect_specials_roll_out(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_set_values(&anim, 0, 100);
    lv_anim_set_delay(&anim, anim_args->delay);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)handle_roll_out);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_time(&anim, anim_args->duration);
    lv_anim_start(&anim);
}

#endif /*LV_USE_ANIM_EFFECT*/
