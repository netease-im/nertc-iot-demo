#include "lv_anim_effect_fade.h"
#if LV_USE_ANIM_EFFECT

static void translate_x(lv_obj_t *obj, lv_anim_effect_args_t *anim_args, int32_t start, int32_t end)
{
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_effect_translate_x(obj, start);
    lv_anim_set_values(&anim, start, end);
    lv_anim_set_delay(&anim, anim_args->delay);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_anim_effect_translate_x);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_time(&anim, anim_args->duration);
    lv_anim_start(&anim);
}
void translate_y(lv_obj_t *obj, lv_anim_effect_args_t *anim_args, int32_t start, int32_t end)
{
    // y轴
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_effect_translate_y(obj, start);
    lv_anim_set_values(&anim, start, end);
    lv_anim_set_delay(&anim, anim_args->delay);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_anim_effect_translate_y);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_time(&anim, anim_args->duration);
    lv_anim_start(&anim);
}

void set_opa(lv_obj_t *obj, lv_anim_effect_args_t *anim_args, bool in)
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

void lv_anim_effect_fade_in(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{

    // 透明
    set_opa(obj, anim_args, true);
}
void lv_anim_effect_fade_in_bottom_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明
    set_opa(obj, anim_args, true);
    // y轴
    translate_y(obj, anim_args, lv_obj_get_style_height(obj, 0), 0);
    // x轴
    translate_x(obj, anim_args, -lv_obj_get_style_width(obj, 0), 0);
}
void lv_anim_effect_fade_in_bottom_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明
    set_opa(obj, anim_args, true);

    // y轴
    translate_y(obj, anim_args, lv_obj_get_style_height(obj, 0), 0);

    // x轴
    translate_x(obj, anim_args, lv_obj_get_style_width(obj, 0), 0);
}
void lv_anim_effect_fade_in_down(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    set_opa(obj, anim_args, true);
    translate_y(obj, anim_args, -lv_obj_get_style_height(obj, 0), 0);
}
void lv_anim_effect_fade_in_down_big(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    set_opa(obj, anim_args, true);
    translate_y(obj, anim_args, -(lv_obj_get_style_x(obj, 0) << 1), 0);
}
void lv_anim_effect_fade_in_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明
    set_opa(obj, anim_args, true);
    // x轴
    translate_x(obj, anim_args, -lv_obj_get_style_width(obj, 0), 0);
}
void lv_anim_effect_fade_in_left_big(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明
    set_opa(obj, anim_args, true);
    // x轴
    translate_x(obj, anim_args, -(lv_obj_get_style_x(obj, 0) << 1), 0);
}
void lv_anim_effect_fade_in_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明
    set_opa(obj, anim_args, true);
    // x轴
    translate_x(obj, anim_args, lv_obj_get_style_width(obj, 0), 0);
}
void lv_anim_effect_fade_in_right_big(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明
    set_opa(obj, anim_args, true);
    // x轴
    lv_disp_t *d = lv_obj_get_disp(obj);
    int32_t translate = lv_disp_get_hor_res(d) - lv_obj_get_style_x(obj, 0); // 屏幕宽度-控件x轴坐标
    translate_x(obj, anim_args, (translate << 1), 0);
}
void lv_anim_effect_fade_in_top_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明
    set_opa(obj, anim_args, true);
    // y轴
    translate_y(obj, anim_args, -lv_obj_get_style_height(obj, 0), 0);
    // x轴
    translate_x(obj, anim_args, -lv_obj_get_style_width(obj, 0), 0);
}
void lv_anim_effect_fade_in_top_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明
    set_opa(obj, anim_args, true);
    // y轴
    translate_y(obj, anim_args, -lv_obj_get_style_height(obj, 0), 0);
    // x轴
    translate_x(obj, anim_args, lv_obj_get_style_width(obj, 0), 0);
}
void lv_anim_effect_fade_in_up(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明
    set_opa(obj, anim_args, true);
    // y轴
    translate_y(obj, anim_args, lv_obj_get_style_height(obj, 0), 0);
}
void lv_anim_effect_fade_in_up_big(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    set_opa(obj, anim_args, true);
    lv_disp_t *d = lv_obj_get_disp(obj);
    int32_t translate = lv_disp_get_ver_res(d) - lv_obj_get_style_y(obj, 0); // 屏幕宽度-控件x轴坐标
    translate_y(obj, anim_args, translate << 1, 0);
}

void lv_anim_effect_fade_out(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明
    set_opa(obj, anim_args, false);
}
void lv_anim_effect_fade_out_bottom_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明
    set_opa(obj, anim_args, false);
    // y轴
    translate_y(obj, anim_args, 0, lv_obj_get_style_height(obj, 0));
    // x轴
    translate_x(obj, anim_args, 0, -lv_obj_get_style_width(obj, 0));
}
void lv_anim_effect_fade_out_bottom_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明
    set_opa(obj, anim_args, false);
    // y轴
    translate_y(obj, anim_args, 0, lv_obj_get_style_height(obj, 0));
    // x轴
    translate_x(obj, anim_args, 0, lv_obj_get_style_width(obj, 0));
}
void lv_anim_effect_fade_out_down(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明
    set_opa(obj, anim_args, false);
    // y轴
    translate_y(obj, anim_args, 0, lv_obj_get_style_height(obj, 0));
}
void lv_anim_effect_fade_out_down_big(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    set_opa(obj, anim_args, false);
    lv_disp_t *d = lv_obj_get_disp(obj);
    int32_t translate = lv_disp_get_ver_res(d) - lv_obj_get_style_y(obj, 0); // 屏幕宽度-控件x轴坐标
    translate = translate << 1;
    translate_y(obj, anim_args, 0, translate);
}
void lv_anim_effect_fade_out_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明
    set_opa(obj, anim_args, false);
    // x轴
    translate_x(obj, anim_args, 0, -lv_obj_get_style_width(obj, 0));
}
void lv_anim_effect_fade_out_left_big(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明
    set_opa(obj, anim_args, false);
    // x轴
    translate_x(obj, anim_args, 0, -(lv_obj_get_style_x(obj, 0) << 1));
}
void lv_anim_effect_fade_out_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明
    set_opa(obj, anim_args, false);
    // x轴
    translate_x(obj, anim_args, 0, lv_obj_get_style_width(obj, 0));
}
void lv_anim_effect_fade_out_right_big(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明
    set_opa(obj, anim_args, false);
    // x轴
    lv_disp_t *d = lv_obj_get_disp(obj);
    int32_t translate = lv_disp_get_hor_res(d) - lv_obj_get_style_x(obj, 0); // 屏幕宽度-控件x轴坐标
    translate_x(obj, anim_args, 0, (translate << 1));
}
void lv_anim_effect_fade_out_top_left(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明
    set_opa(obj, anim_args, false);
    // y轴
    translate_y(obj, anim_args, 0, -lv_obj_get_style_height(obj, 0));
    // x轴
    translate_x(obj, anim_args, 0, -lv_obj_get_style_width(obj, 0));
}
void lv_anim_effect_fade_out_top_right(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    // 透明
    set_opa(obj, anim_args, false);
    // y轴
    translate_y(obj, anim_args, 0, -lv_obj_get_style_height(obj, 0));
    // x轴
    translate_x(obj, anim_args, 0, lv_obj_get_style_width(obj, 0));
}
void lv_anim_effect_fade_out_up(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    set_opa(obj, anim_args, false);
    translate_y(obj, anim_args, 0, -lv_obj_get_style_height(obj, 0));
}
void lv_anim_effect_fade_out_up_big(lv_obj_t *obj, lv_anim_effect_args_t *anim_args)
{
    set_opa(obj, anim_args, false);
    lv_disp_t *d = lv_obj_get_disp(obj);
    int32_t translate = lv_disp_get_ver_res(d) - lv_obj_get_style_y(obj, 0); // 屏幕宽度-控件x轴坐标
    translate_y(obj, anim_args, 0, -(translate << 1));
}

#endif /*LV_USE_ANIM_EFFECT*/
