#include "lv_anim_effect_zooming.h"
#if LV_USE_ANIM_EFFECT

static void custom_scale(lv_anim_t *anim, int32_t val)
{
    lv_obj_t *obj = anim->user_data;
    lv_anim_effect_scale(obj, val);
}

static void custom_opa(lv_anim_t *anim, int32_t val)
{
    lv_obj_t *obj = anim->user_data;
    lv_anim_effect_set_opa(obj, val);
}

static void set_scale(lv_obj_t *obj, int32_t delay, int32_t time, int32_t start, int32_t end, void *path_cb)
{

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_set_values(&anim, start, end);
    lv_anim_set_delay(&anim, delay);
    lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)custom_scale);
    lv_anim_set_path_cb(&anim, path_cb);
    lv_anim_set_user_data(&anim, obj);
    lv_anim_set_time(&anim, time);
    lv_anim_start(&anim);
}

static void set_opa(lv_obj_t *obj, int32_t delay, int32_t time, int32_t start, int32_t end, void *path_cb)
{

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_set_values(&anim, start, end);
    lv_anim_set_delay(&anim, delay);
    lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)custom_opa);
    lv_anim_set_path_cb(&anim, path_cb);
    lv_anim_set_user_data(&anim, obj);
    lv_anim_set_time(&anim, time);
    lv_anim_start(&anim);
}

static void in_scale_and_translate_handle(lv_obj_t *obj, int32_t scale, int32_t translate_x, int32_t translate_y)
{
    int32_t h = lv_obj_get_style_height(obj, 0);
    int32_t retH = h * scale / 100;
    int32_t w = lv_obj_get_style_width(obj, 0);
    int32_t retW = w * scale / 100;
    int32_t tranX = (retW - w) / 2;
    int32_t tranY = (retH - h) / 2;
    lv_obj_set_style_translate_x(obj, -tranX + translate_x, LV_PART_MAIN);
    lv_obj_set_style_translate_y(obj, -tranY + translate_y, LV_PART_MAIN);
    if (lv_obj_check_type(obj, &lv_img_class)) {
        // 因为图片控件不支持单独的x或y轴缩放，所以暂不处理
        lv_img_set_zoom(obj, (uint16_t)scale * 2.56); // 256为一倍
    } else {
        lv_obj_set_style_transform_zoom(obj, (uint16_t)scale * 2.56, 0); // 如果图片控件缩放会导致崩溃
    }
}

static void down_in_one(lv_anim_t *anim, int32_t v)
{
    lv_obj_t *obj = anim->user_data;
    int32_t h = lv_obj_get_style_height(obj, 0);
    uint32_t scale = v * 37.5 / 100 + 10;
    uint32_t tranlate = -h + v * h * 1.2 / 100;
    in_scale_and_translate_handle(obj, scale, 0, tranlate);
}

static void down_in_two(lv_anim_t *anim, int32_t v)
{

    lv_obj_t *obj = anim->user_data;
    int32_t h = lv_obj_get_style_height(obj, 0);
    uint32_t scale = v * 48 / 100 + 52;
    uint32_t tranlate = h * 1.2 - v * h * 1.2 / 100;
    in_scale_and_translate_handle(obj, scale, 0, tranlate);
}

static void up_in_one(lv_anim_t *anim, int32_t v)
{
    lv_obj_t *obj = anim->user_data;
    int32_t h = lv_obj_get_style_height(obj, 0);
    uint32_t scale = v * 37.5 / 100 + 10;
    uint32_t tranlate = h - v * h * 1.5 / 100;
    in_scale_and_translate_handle(obj, scale, 0, tranlate);
}

static void up_in_two(lv_anim_t *anim, int32_t v)
{
    lv_obj_t *obj = anim->user_data;
    int32_t h = lv_obj_get_style_height(obj, 0);
    uint32_t scale = v * 48 / 100 + 52;
    uint32_t tranlate = v * h * 0.5 / 100 - h * 0.5;
    in_scale_and_translate_handle(obj, scale, 0, tranlate);
}

static void left_in_one(lv_anim_t *anim, int32_t v)
{
    lv_obj_t *obj = anim->user_data;
    uint32_t scale = v * 37.5 / 100 + 10;
    int32_t w = lv_obj_get_style_width(obj, 0);
    uint32_t tranlate = -v * w / 100 + 10;
    in_scale_and_translate_handle(obj, scale, tranlate, 0);
}

static void left_in_two(lv_anim_t *anim, int32_t v)
{
    lv_obj_t *obj = anim->user_data;
    int32_t h = lv_obj_get_style_height(obj, 0);
    uint32_t scale = v * 48 / 100 + 52;
    int32_t w = lv_obj_get_style_width(obj, 0);
    uint32_t tranlate = 10 - v * 10 / 100;
    in_scale_and_translate_handle(obj, scale, tranlate, 0);
}

static void right_in_one(lv_anim_t *anim, int32_t v)
{
    lv_obj_t *obj = anim->user_data;
    int32_t h = lv_obj_get_style_height(obj, 0);
    uint32_t scale = v * 37.5 / 100 + 10;
    int32_t w = lv_obj_get_style_width(obj, 0);
    uint32_t tranlate = v * w / 100 - 10;
    in_scale_and_translate_handle(obj, scale, tranlate, 0);
}

static void right_in_two(lv_anim_t *anim, int32_t v)
{
    lv_obj_t *obj = anim->user_data;
    int32_t h = lv_obj_get_style_height(obj, 0);
    uint32_t scale = v * 48 / 100 + 52;
    int32_t w = lv_obj_get_style_width(obj, 0);
    uint32_t tranlate = -10 + v * 10 / 100;
    in_scale_and_translate_handle(obj, scale, tranlate, 0);
}

static void lv_anim_effect_zooming_in_by_type(lv_obj_t *obj, lv_anim_effect_args_t *anim_args, uint8_t type)
{

    int32_t time = lv_anim_effect_get_time(anim_args->duration, 60);
    // 透明度

    set_opa(obj, anim_args->delay, time, LV_OPA_0, LV_OPA_100, lv_anim_path_ease_in);
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_set_values(&anim, 0, 100);
    lv_anim_set_delay(&anim, anim_args->delay);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in);
    lv_anim_set_user_data(&anim, obj);
    lv_anim_set_time(&anim, time);

    lv_anim_t anim1;
    lv_anim_init(&anim1);
    lv_anim_set_var(&anim1, obj);
    lv_anim_set_early_apply(&anim1, false);
    lv_anim_set_values(&anim1, 0, 100);
    lv_anim_set_delay(&anim1, anim_args->delay + time);
    lv_anim_set_path_cb(&anim1, lv_anim_path_ease_out);
    lv_anim_set_user_data(&anim1, obj);
    lv_anim_set_time(&anim1, anim_args->duration - time);
    switch (type) {
    case 1:
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)down_in_one);
        lv_anim_set_custom_exec_cb(&anim1, (lv_anim_custom_exec_cb_t)down_in_two);
        break;
    case 2:
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)left_in_one);
        lv_anim_set_custom_exec_cb(&anim1, (lv_anim_custom_exec_cb_t)left_in_two);
        break;
    case 3:
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)right_in_one);
        lv_anim_set_custom_exec_cb(&anim1, (lv_anim_custom_exec_cb_t)right_in_two);
        break;
    case 4:
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)up_in_one);
        lv_anim_set_custom_exec_cb(&anim1, (lv_anim_custom_exec_cb_t)up_in_two);
        break;
    default:
        break;
    }

    lv_anim_start(&anim);
    lv_anim_start(&anim1);
}

void lv_anim_effect_zooming_in(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{

    int32_t t_50 = lv_anim_effect_get_time(anim_args->duration, 50);
    // 透明度
    // lv_anim_effect_set_opa(obj, LV_OPA_0);
    set_opa(obj, anim_args->delay, t_50, LV_OPA_0, LV_OPA_100, lv_anim_path_linear);
    // 缩放
    set_scale(obj, anim_args->delay, anim_args->duration, 30, 100, lv_anim_path_linear);
}

void lv_anim_effect_zooming_in_down(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    lv_anim_effect_zooming_in_by_type(obj, anim_args, 1);
}

void lv_anim_effect_zooming_in_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{

    lv_anim_effect_zooming_in_by_type(obj, anim_args, 2);
}

void lv_anim_effect_zooming_in_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{

    lv_anim_effect_zooming_in_by_type(obj, anim_args, 3);
}

void lv_anim_effect_zooming_in_up(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{

    lv_anim_effect_zooming_in_by_type(obj, anim_args, 4);
}

static void down_out_one(lv_anim_t *anim, int32_t v)
{
    lv_obj_t *obj = anim->user_data;
    int32_t h = lv_obj_get_style_height(obj, 0);
    uint32_t scale = 100 - v * 52 / 100;
    uint32_t tranlate = -v * h / 2 / 100;
    in_scale_and_translate_handle(obj, scale, 0, tranlate);
}

static void down_out_two(lv_anim_t *anim, int32_t v)
{

    lv_obj_t *obj = anim->user_data;
    int32_t h = lv_obj_get_style_height(obj, 0);
    uint32_t scale = 100 - 52 - v * 38 / 100;
    uint32_t tranlate = -h / 2 + h * 2 * v / 100;
    in_scale_and_translate_handle(obj, scale, 0, tranlate);
}

static void up_out_one(lv_anim_t *anim, int32_t v)
{
    lv_obj_t *obj = anim->user_data;
    int32_t h = lv_obj_get_style_height(obj, 0);
    uint32_t scale = 100 - v * 52 / 100;
    uint32_t tranlate = h >> 1;
    in_scale_and_translate_handle(obj, scale, 0, tranlate);
}

static void up_out_two(lv_anim_t *anim, int32_t v)
{
    lv_obj_t *obj = anim->user_data;
    int32_t h = lv_obj_get_style_height(obj, 0);
    uint32_t scale = 100 - 52 - v * 38 / 100;
    uint32_t halfH = h >> 1;
    uint32_t tranlate = -v * h * 2 / 100 + halfH;
    in_scale_and_translate_handle(obj, scale, 0, tranlate);
}

static void left_out_one(lv_anim_t *anim, int32_t v)
{
    lv_obj_t *obj = anim->user_data;
    uint32_t scale = 100 - v * 52 / 100;
    int32_t w = lv_obj_get_style_width(obj, 0);
    uint32_t tranlate = 42 * v / 100;
    in_scale_and_translate_handle(obj, scale, tranlate, 0);
}

static void left_out_two(lv_anim_t *anim, int32_t v)
{
    lv_obj_t *obj = anim->user_data;
    int32_t h = lv_obj_get_style_height(obj, 0);
    uint32_t scale = 100 - 52 - v * 38 / 100;
    int32_t w = lv_obj_get_style_width(obj, 0);
    uint32_t tranlate = -v * w * 2 / 100 + 42;
    in_scale_and_translate_handle(obj, scale, tranlate, 0);
}

static void right_out_one(lv_anim_t *anim, int32_t v)
{
    lv_obj_t *obj = anim->user_data;
    int32_t h = lv_obj_get_style_height(obj, 0);
    uint32_t scale = 100 - v * 52 / 100;
    int32_t w = lv_obj_get_style_width(obj, 0);
    uint32_t tranlate = v * -42 / 100;
    in_scale_and_translate_handle(obj, scale, tranlate, 0);
}

static void right_out_two(lv_anim_t *anim, int32_t v)
{
    lv_obj_t *obj = anim->user_data;
    int32_t h = lv_obj_get_style_height(obj, 0);
    uint32_t scale = 100 - 52 - v * 38 / 100;
    int32_t w = lv_obj_get_style_width(obj, 0);
    uint32_t tranlate = -42 + v * w * 2 / 100;
    in_scale_and_translate_handle(obj, scale, tranlate, 0);
}

static void lv_anim_effect_zooming_out_by_type(lv_obj_t *obj, lv_anim_effect_args_t *anim_args, uint8_t type)
{

    int32_t time = lv_anim_effect_get_time(anim_args->duration, 40);
    // 透明度
    set_opa(obj, anim_args->delay + time, anim_args->duration - time, LV_OPA_100, LV_OPA_0, lv_anim_path_ease_out);
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_early_apply(&anim, false);
    lv_anim_set_values(&anim, 0, 100);
    lv_anim_set_delay(&anim, anim_args->delay);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
    lv_anim_set_user_data(&anim, obj);
    lv_anim_set_time(&anim, time);
    lv_anim_t anim1;
    lv_anim_init(&anim1);
    lv_anim_set_var(&anim1, obj);
    lv_anim_set_early_apply(&anim1, false);
    lv_anim_set_values(&anim1, 0, 100);
    lv_anim_set_delay(&anim1, anim_args->delay + time + 33 * 2);
    lv_anim_set_path_cb(&anim1, lv_anim_path_ease_in);
    lv_anim_set_user_data(&anim1, obj);
    lv_anim_set_time(&anim1, anim_args->duration - time - 33 * 2);
    switch (type) {
    case 1:
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)down_out_one);
        lv_anim_set_custom_exec_cb(&anim1, (lv_anim_custom_exec_cb_t)down_out_two);
        break;
    case 2:
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)left_out_one);
        lv_anim_set_custom_exec_cb(&anim1, (lv_anim_custom_exec_cb_t)left_out_two);
        break;
    case 3:
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)right_out_one);
        lv_anim_set_custom_exec_cb(&anim1, (lv_anim_custom_exec_cb_t)right_out_two);
        break;
    case 4:
        lv_anim_set_custom_exec_cb(&anim, (lv_anim_custom_exec_cb_t)up_out_one);
        lv_anim_set_custom_exec_cb(&anim1, (lv_anim_custom_exec_cb_t)up_out_two);
        break;
    default:
        break;
    }

    lv_anim_start(&anim);
    lv_anim_start(&anim1);
}

void lv_anim_effect_zooming_out(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    int32_t t_50 = lv_anim_effect_get_time(anim_args->duration, 50);
    // 透明度
    set_opa(obj, anim_args->delay, t_50, LV_OPA_100, LV_OPA_0, lv_anim_path_linear);
    // 缩放
    set_scale(obj, anim_args->delay, t_50, 100, 30, lv_anim_path_linear);
}

void lv_anim_effect_zooming_out_down(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    lv_anim_effect_zooming_out_by_type(obj, anim_args, 1);
}

void lv_anim_effect_zooming_out_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    lv_anim_effect_zooming_out_by_type(obj, anim_args, 2);
}

void lv_anim_effect_zooming_out_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    lv_anim_effect_zooming_out_by_type(obj, anim_args, 3);
}

void lv_anim_effect_zooming_out_up(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    lv_anim_effect_zooming_out_by_type(obj, anim_args, 4);
}

#endif /*LV_USE_ANIM_EFFECT*/
