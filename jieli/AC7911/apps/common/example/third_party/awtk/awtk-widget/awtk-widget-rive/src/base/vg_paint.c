/**
 * File: vg_paint.c
 * Author: AWTK Develop Team
 * Brief:  vector graphic paint
 *
 * Copyright (c) 2021 - 2021 Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2021-06-20 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "vg_paint.h"

vg_paint_t *vg_paint_create(void)
{
    vg_paint_t *paint = TKMEM_ZALLOC(vg_paint_t);
    return_value_if_fail(paint != NULL, NULL);

    return paint;
}

ret_t vg_paint_set_type(vg_paint_t *paint, vg_paint_type_t type)
{
    return_value_if_fail(paint != NULL, RET_BAD_PARAMS);
    paint->type = type;

    return RET_OK;
}

ret_t vg_paint_set_join_type(vg_paint_t *paint, vg_join_type_t join)
{
    return_value_if_fail(paint != NULL, RET_BAD_PARAMS);
    paint->join = join;

    return RET_OK;
}

ret_t vg_paint_set_cap_type(vg_paint_t *paint, vg_cap_type_t cap)
{
    return_value_if_fail(paint != NULL, RET_BAD_PARAMS);
    paint->cap = cap;

    return RET_OK;
}

ret_t vg_paint_set_blend_mode(vg_paint_t *paint, vg_blend_mode_t blend)
{
    return_value_if_fail(paint != NULL, RET_BAD_PARAMS);
    paint->blend_mode = blend;

    return RET_OK;
}

ret_t vg_paint_set_color(vg_paint_t *paint, color_t color)
{
    return_value_if_fail(paint != NULL, RET_BAD_PARAMS);
    paint->color = color;

    return RET_OK;
}

ret_t vg_paint_set_line_width(vg_paint_t *paint, float line_width)
{
    return_value_if_fail(paint != NULL, RET_BAD_PARAMS);
    paint->line_width = line_width;

    return RET_OK;
}

ret_t vg_paint_set_linear_gradient(vg_paint_t *paint, float sx, float sy, float ex, float ey)
{
    return_value_if_fail(paint != NULL, RET_BAD_PARAMS);

    if (paint->gradient != NULL) {
        vg_gradient_destroy(paint->gradient);
        paint->gradient = NULL;
    }

    paint->gradient = vg_gradient_create_linear(sx, sy, ex, ey);

    return RET_OK;
}

ret_t vg_paint_set_radial_gradient(vg_paint_t *paint, float sx, float sy, float ex, float ey)
{
    float dx = ex - sx;
    float dy = ey - sy;
    float r  = sqrt(dx * dx + dy * dy);
    return_value_if_fail(paint != NULL, RET_BAD_PARAMS);

    if (paint->gradient != NULL) {
        vg_gradient_destroy(paint->gradient);
        paint->gradient = NULL;
    }

    paint->gradient = vg_gradient_create_radial(sx, sy, 0, sx, sy, r);

    return RET_OK;
}

ret_t vg_paint_add_gradient_stop(vg_paint_t *paint, color_t color, float stop)
{
    return_value_if_fail(paint != NULL && paint->gradient != NULL, RET_BAD_PARAMS);

    return vg_gradient_add_stop(paint->gradient, color, stop);
}

ret_t vg_paint_destroy(vg_paint_t *paint)
{
    return_value_if_fail(paint != NULL, RET_BAD_PARAMS);
    if (paint->gradient != NULL) {
        vg_gradient_destroy(paint->gradient);
    }
    TKMEM_FREE(paint);

    return RET_OK;
}
