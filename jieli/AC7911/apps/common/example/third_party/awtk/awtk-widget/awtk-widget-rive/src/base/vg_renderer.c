
/**
 * File: vg_renderer.c
 * Author: AWTK Develop Team
 * Brief:  vector graphic renderer
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
#include "vg_renderer.h"

vg_renderer_t *vg_renderer_create(vgcanvas_t *vg)
{
    vg_renderer_t *renderer = NULL;
    return_value_if_fail(vg != NULL, NULL);
    renderer = TKMEM_ZALLOC(vg_renderer_t);
    return_value_if_fail(renderer != NULL, NULL)

    renderer->vg = vg;

    return renderer;
}

ret_t vg_renderer_save(vg_renderer_t *renderer)
{
    return_value_if_fail(renderer != NULL, RET_BAD_PARAMS);
    return vgcanvas_save(renderer->vg);
}

ret_t vg_renderer_restore(vg_renderer_t *renderer)
{
    return_value_if_fail(renderer != NULL, RET_BAD_PARAMS);
    return vgcanvas_restore(renderer->vg);
}

static ret_t vgcanvas_apply_paint(vgcanvas_t *vg, const vg_paint_t *paint)
{
    vg_gradient_t *g = paint->gradient;
    if (paint->type == VG_PAINT_FILL) {
        if (g != NULL && g->gradient.nr > 1) {
            vgcanvas_set_fill_gradient(vg, g);
        } else {
            vgcanvas_set_fill_color(vg, paint->color);
        }
    } else {
        if (g != NULL && g->gradient.nr > 1) {
            vgcanvas_set_stroke_gradient(vg, g);
        } else {
            vgcanvas_set_stroke_color(vg, paint->color);
        }
    }
    vgcanvas_set_line_width(vg, paint->line_width);

    return RET_OK;
}

static ret_t vgcanvas_draw_path(vgcanvas_t *vg, const vg_path_t *path)
{
    const vg_path_t *iter = path;

    vgcanvas_begin_path(vg);
    while (iter != NULL) {
        switch (iter->type) {
        case VG_PATH_MOVE_TO: {
            vgcanvas_move_to(vg, iter->u.move_to.x, iter->u.move_to.y);
            break;
        }
        case VG_PATH_LINE_TO: {
            vgcanvas_line_to(vg, iter->u.line_to.x, iter->u.line_to.y);
            break;
        }
        case VG_PATH_CUBIC_TO: {
            vgcanvas_bezier_to(vg, iter->u.cubic_to.ox, iter->u.cubic_to.oy, iter->u.cubic_to.ix,
                               iter->u.cubic_to.iy, iter->u.cubic_to.x, iter->u.cubic_to.y);
            break;
        }
        case VG_PATH_CLOSE: {
            vgcanvas_close_path(vg);
            break;
        }
        default:
            break;
        }

        iter = iter->next;
    }

    return RET_OK;
}

ret_t vg_renderer_draw_path(vg_renderer_t *renderer, const vg_path_t *path,
                            const vg_paint_t *paint)
{
    vgcanvas_t *vg = NULL;
    return_value_if_fail(renderer != NULL && paint != NULL, RET_BAD_PARAMS);

    if (path == NULL) {
        return RET_FAIL;
    }

    vg = renderer->vg;

    vgcanvas_apply_paint(vg, paint);
    vgcanvas_draw_path(vg, path);

    if (paint->type == VG_PAINT_FILL) {
        vgcanvas_fill(vg);
    } else {
        vgcanvas_stroke(vg);
    }

    return RET_OK;
}

ret_t vg_renderer_clip_path(vg_renderer_t *renderer, const vg_path_t *path)
{
    rect_t r;
    ret_t ret = RET_FAIL;
    return_value_if_fail(renderer != NULL && path != NULL, RET_BAD_PARAMS);

    if (renderer->vg->vt->clip_path != NULL) {
        vgcanvas_draw_path(renderer->vg, path);
        ret = vgcanvas_clip_path(renderer->vg);
        vgcanvas_begin_path(renderer->vg);
    } else if (vg_path_is_rect(path, &r)) {
        ret = vgcanvas_clip_rect(renderer->vg, r.x, r.y, r.w, r.h);
    }

    return ret;
}

ret_t vg_renderer_transform(vg_renderer_t *renderer, float a, float b, float c, float d, float e,
                            float f)
{
    return_value_if_fail(renderer != NULL, RET_BAD_PARAMS);

    return vgcanvas_transform(renderer->vg, a, b, c, d, e, f);
}

ret_t vg_renderer_destroy(vg_renderer_t *renderer)
{
    return_value_if_fail(renderer != NULL, RET_BAD_PARAMS);
    renderer->vg = NULL;
    TKMEM_FREE(renderer);

    return RET_OK;
}
