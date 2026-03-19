/**
 * File: vg_path.h
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

#ifndef TK_VG_PATH_H
#define TK_VG_PATH_H

#include "tkc/rect.h"
#include "tkc/matrix.h"

BEGIN_C_DECLS

/**
 * @enum vg_path_type_t
 * VG路径类型。
 */
typedef enum _vg_path_type_t {
    /**
     * @const VG_PATH_NONE
     * 无效类型。
     */
    VG_PATH_NONE,
    /**
     * @const VG_PATH_MOVE_TO
     * 移动到指定点。
     */
    VG_PATH_MOVE_TO,
    /**
     * @const VG_PATH_LINE_TO
     * 连线到指定点。
     */
    VG_PATH_LINE_TO,
    /**
     * @const VG_PATH_CUBIC_TO
     * 连曲线到指定点。
     */
    VG_PATH_CUBIC_TO,
    /**
     * @const VG_PATH_CLOSE
     * 封闭路径。
     */
    VG_PATH_CLOSE
} vg_path_type_t;

/**
 * @class vg_move_to_t
 * 移动到指定点。
 */
typedef struct _vg_move_to_t {
    /**
     * @property {float} x
     * @annotation ["readable"]
     * x坐标。
     */
    float x;
    /**
     * @property {float} y
     * @annotation ["readable"]
     * y坐标。
     */
    float y;
} vg_move_to_t;

/**
 * @class vg_line_to_t
 * 连线到指定点。
 */
typedef struct _vg_line_to_t {
    /**
     * @property {float} x
     * @annotation ["readable"]
     * x坐标。
     */
    float x;
    /**
     * @property {float} y
     * @annotation ["readable"]
     * y坐标。
     */
    float y;
} vg_line_to_t;

/**
 * @class vg_cubic_to_t
 * 连曲线到指定点。
 */
typedef struct _vg_cubic_to_t {
    /**
     * @property {float} ox
     * @annotation ["readable"]
     * x坐标。
     */
    float ox;
    /**
     * @property {float} oy
     * @annotation ["readable"]
     * y坐标。
     */
    float oy;
    /**
     * @property {float} ix
     * @annotation ["readable"]
     * x坐标。
     */
    float ix;
    /**
     * @property {float} iy
     * @annotation ["readable"]
     * y坐标。
     */
    float iy;
    /**
     * @property {float} x
     * @annotation ["readable"]
     * x坐标。
     */
    float x;
    /**
     * @property {float} y
     * @annotation ["readable"]
     * y坐标。
     */
    float y;
} vg_cubic_to_t;

struct _vg_path_t;
typedef struct _vg_path_t vg_path_t;

/**
 * @class vg_cubic_to_t
 * 连曲线到指定点。
 */
struct _vg_path_t {
    /**
     * @property {vg_path_type_t} type
     * @annotation ["readable"]
     * 类型。
     */
    vg_path_type_t type;

    union {
        vg_move_to_t move_to;
        vg_line_to_t line_to;
        vg_cubic_to_t cubic_to;
    } u;

    /**
     * @property {vg_path_t*} next
     * @annotation ["readable"]
     * 下一段路径。
     */
    vg_path_t *next;
};

/**
 * @method vg_path_add_move_to
 * 追加move_to路径。
 * @param {vg_path_t*} first 首条路径。
 * @param {float} x x坐标。
 * @param {float} y y坐标。
 *
 * @return {vg_path_t*} 首条路径对象。
 */
vg_path_t *vg_path_add_move_to(vg_path_t *first, float x, float y);

/**
 * @method vg_path_add_line_to
 * 追加line_to路径。
 * @param {vg_path_t*} first 首条路径。
 * @param {float} x x坐标。
 * @param {float} y y坐标。
 *
 * @return {vg_path_t*} 首条路径对象。
 */
vg_path_t *vg_path_add_line_to(vg_path_t *first, float x, float y);

/**
 * @method vg_path_add_cubic_to
 * 追加cubic_to路径。
 * @param {vg_path_t*} first 首条路径。
 * @param {float} ox x坐标。
 * @param {float} oy y坐标。
 * @param {float} ix x坐标。
 * @param {float} iy y坐标。
 * @param {float} x x坐标。
 * @param {float} y y坐标。
 *
 * @return {vg_path_t*} 首条路径对象。
 */
vg_path_t *vg_path_add_cubic_to(vg_path_t *first, float ox, float oy, float ix, float iy, float x,
                                float y);

/**
 * @method vg_path_add_close
 * 追加close路径。
 * @param {vg_path_t*} first 首条路径。
 *
 * @return {vg_path_t*} 首条路径对象。
 */
vg_path_t *vg_path_add_close(vg_path_t *first);

/**
 * @method vg_path_dup
 * 复制路径。
 * @param {const vg_path_t*} first 首条路径。
 * @param {bool_t} all 是否复制全部。
 *
 * @return {vg_path_t*} 新的路径对象。
 */
vg_path_t *vg_path_dup(const vg_path_t *first, bool_t all);

/**
 * @method vg_path_append
 * 追加路径。
 * @param {vg_path_t*} first 首条路径。
 * @param {vg_path_t*} path 路径。
 *
 * @return {vg_path_t*} 首条路径对象。
 */
vg_path_t *vg_path_append(vg_path_t *first, vg_path_t *path);

/**
 * @method vg_path_is_rect
 * 是否是rect。
 * @param {vg_path_t*} first 首条路径。
 * @param {rect_t*} result 如果是rect，用于返回rect。
 *
 * @return {bool_t} 返回TRUE表示是，否则表示不是。
 */
bool_t vg_path_is_rect(const vg_path_t *first, rect_t *result);

/**
 * @method vg_path_transform
 * 变换。
 * @param {vg_path_t*} first 首条路径。
 * @param {matrix_t*} m 矩阵。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t vg_path_transform(vg_path_t *first, matrix_t *m);

/**
 * @method vg_path_destroy
 * 销毁路径。
 * @param {vg_path_t*} first 首条路径。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t vg_path_destroy(vg_path_t *first);

END_C_DECLS

#endif /*TK_VG_PATH_H*/
