/**
 * File: vg_path.c
 * Author: AWTK Develop Team
 * Brief:  vector graphic path
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

#include "vg_path.h"
#include "tkc/mem.h"

vg_path_t *vg_path_append(vg_path_t *first, vg_path_t *path)
{
    vg_path_t *iter = first;
    return_value_if_fail(path != NULL, first);

    if (first == NULL) {
        return path;
    }

    while (iter->next != NULL) {
        iter = iter->next;
    }
    iter->next = path;

    return first;
}

vg_path_t *vg_path_add_move_to(vg_path_t *first, float x, float y)
{
    vg_path_t *path = TKMEM_ZALLOC(vg_path_t);
    return_value_if_fail(path != NULL, first);

    path->u.move_to.x = x;
    path->u.move_to.y = y;
    path->type = VG_PATH_MOVE_TO;

    return vg_path_append(first, path);
}

vg_path_t *vg_path_add_line_to(vg_path_t *first, float x, float y)
{
    vg_path_t *path = TKMEM_ZALLOC(vg_path_t);
    return_value_if_fail(path != NULL, first);

    path->u.line_to.x = x;
    path->u.line_to.y = y;
    path->type = VG_PATH_LINE_TO;

    return vg_path_append(first, path);
}

vg_path_t *vg_path_add_cubic_to(vg_path_t *first, float ox, float oy, float ix, float iy, float x,
                                float y)
{
    vg_path_t *path = TKMEM_ZALLOC(vg_path_t);
    return_value_if_fail(path != NULL, first);

    path->u.cubic_to.ox = ox;
    path->u.cubic_to.oy = oy;
    path->u.cubic_to.ix = ix;
    path->u.cubic_to.iy = iy;
    path->u.cubic_to.x = x;
    path->u.cubic_to.y = y;
    path->type = VG_PATH_CUBIC_TO;

    return vg_path_append(first, path);
}

vg_path_t *vg_path_add_close(vg_path_t *first)
{
    vg_path_t *path = TKMEM_ZALLOC(vg_path_t);
    return_value_if_fail(path != NULL, first);

    path->type = VG_PATH_CLOSE;

    return vg_path_append(first, path);
}

vg_path_t *vg_path_dup(const vg_path_t *first, bool_t all)
{
    vg_path_t *dst = NULL;
    vg_path_t *new_first = NULL;
    const vg_path_t *src = first;

    if (first == NULL) {
        return NULL;
    }

    while (src != NULL) {
        vg_path_t *path = TKMEM_ZALLOC(vg_path_t);
        memcpy(path, src, sizeof(vg_path_t));
        path->next = NULL;

        if (dst == NULL) {
            dst = path;
            new_first = path;
            if (!all) {
                break;
            }
        } else {
            dst->next = path;
            dst = path;
        }

        src = src->next;
    }

    return new_first;
}

ret_t vg_path_destroy(vg_path_t *first)
{
    vg_path_t *iter = first;
    vg_path_t *next = first;

    if (first == NULL) {
        return RET_OK;
    }

    while (iter != NULL) {
        next = iter->next;
        TKMEM_FREE(iter);
        iter = next;
    }

    return RET_OK;
}

bool_t vg_path_is_rect(const vg_path_t *first, rect_t *result)
{
    point_t top_l = {0, 0};
    point_t top_r = {0, 0};
    point_t bottom_l = {0, 0};
    point_t bottom_r = {0, 0};
    const vg_path_t *iter = first;

    if (iter == NULL || iter->type != VG_PATH_MOVE_TO) {
        return FALSE;
    }
    top_l.x = iter->u.move_to.x;
    top_l.y = iter->u.move_to.y;
    iter = iter->next;

    if (iter == NULL || iter->type != VG_PATH_LINE_TO) {
        return FALSE;
    }
    top_r.x = iter->u.line_to.x;
    top_r.y = iter->u.line_to.y;

    if (top_l.y != top_r.y) {
        return FALSE;
    }

    iter = iter->next;
    if (iter == NULL || iter->type != VG_PATH_LINE_TO) {
        return FALSE;
    }
    bottom_r.x = iter->u.line_to.x;
    bottom_r.y = iter->u.line_to.y;
    if (bottom_r.x != top_r.x) {
        return FALSE;
    }

    iter = iter->next;
    if (iter == NULL || iter->type != VG_PATH_LINE_TO) {
        return FALSE;
    }
    bottom_l.x = iter->u.line_to.x;
    bottom_l.y = iter->u.line_to.y;

    if (top_l.x != bottom_l.x || bottom_l.y != bottom_r.y) {
        return FALSE;
    }

    iter = iter->next;
    if (iter == NULL || iter->type != VG_PATH_CLOSE) {
        return FALSE;
    }

    if (result != NULL) {
        *result = rect_init(top_l.x, top_l.y, bottom_r.x - top_l.x, bottom_r.y - top_l.y);
    }

    return TRUE;
}

static ret_t transform_pointf(matrix_t *m, float *x, float *y)
{
    matrix_transform_pointf(m, *x, *y, x, y);
    return RET_OK;
}

ret_t vg_path_transform(vg_path_t *first, matrix_t *m)
{
    vg_path_t *iter = first;

    while (iter != NULL) {
        switch (iter->type) {
        case VG_PATH_MOVE_TO: {
            transform_pointf(m, &(iter->u.move_to.x), &(iter->u.move_to.y));
            break;
        }
        case VG_PATH_LINE_TO: {
            transform_pointf(m, &(iter->u.line_to.x), &(iter->u.line_to.y));
            break;
        }
        case VG_PATH_CUBIC_TO: {
            transform_pointf(m, &(iter->u.cubic_to.x), &(iter->u.cubic_to.y));
            transform_pointf(m, &(iter->u.cubic_to.ox), &(iter->u.cubic_to.oy));
            transform_pointf(m, &(iter->u.cubic_to.ix), &(iter->u.cubic_to.iy));
            break;
        }
        default:
            break;
        }
        iter = iter->next;
    }
    return RET_OK;
}
