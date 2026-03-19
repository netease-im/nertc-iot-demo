/**
 * File: vg_paint.h
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

#ifndef TK_VG_PAINT_H
#define TK_VG_PAINT_H

#include "base/vg_gradient.h"

BEGIN_C_DECLS

/**
 * @enum vg_paint_type_t
 * paint类型。
 */
typedef enum _vg_paint_type_t {
    /**
     * @const VG_PAINT_FILL
     * Fill
     */
    VG_PAINT_FILL,
    /**
     * @const VG_PAINT_STROKE
     * Stroke
     */
    VG_PAINT_STROKE
} vg_paint_type_t;

/**
 * @enum vg_join_type_t
 * 交点的类型。
 */
typedef enum _vg_join_type_t {
    /**
     * @const VG_JOIN_MITTER
     * mitter
     */
    VG_JOIN_MITTER = 0,
    /**
     * @const VG_JOIN_ROUND
     * round
     */
    VG_JOIN_ROUND = 1,
    /**
     * @const VG_JOIN_BEVEL
     * bevel
     */
    VG_JOIN_BEVEL = 2
} vg_join_type_t;

/**
 * @enum vg_cap_type_t
 * 顶点的类型。
 */
typedef enum _vg_cap_type_t {
    /**
     * @const VG_CAP_BUTT
     * butt
     */
    VG_CAP_BUTT = 0,
    /**
     * @const VG_CAP_ROUND
     * round
     */
    VG_CAP_ROUND = 1,
    /**
     * @const VG_CAP_SQUARE
     * square
     */
    VG_CAP_SQUARE = 2
} vg_cap_type_t;

/**
 * @enum vg_blend_mode_t
 * 合成模式。
 */
typedef enum _vg_blend_mode_t {
    /**
     * @const VG_BLEND_SRCOVER
     * SRC OVER
     */
    VG_BLEND_SRCOVER = 0,
} vg_blend_mode_t;

struct _vg_paint_t;
typedef struct _vg_paint_t vg_paint_t;

/**
 * @class vg_paint_t
 * paint。
 */
struct _vg_paint_t {
    /**
     * @property {vg_paint_type_t} type
     * @annotation ["readable"]
     * 类型。
     */
    vg_paint_type_t type;

    /**
     * @property {color_t} color
     * @annotation ["readable"]
     * 颜色。
     */
    color_t color;

    /**
     * @property {float} line_width
     * @annotation ["readable"]
     * 线宽。
     */
    float line_width;

    /**
     * @property {vg_join_type_t} join
     * @annotation ["readable"]
     * 交点类型。
     */
    vg_join_type_t join;
    /**
     * @property {vg_cap_type_t} cap
     * @annotation ["readable"]
     * 顶点类型。
     */
    vg_cap_type_t cap;
    /**
     * @property {vg_cap_type_t} blend_mode
     * @annotation ["readable"]
     * 合成模式。
     */
    vg_blend_mode_t blend_mode;

    /**
     * @property {vg_gradient_t*} gradient
     * @annotation ["readable"]
     * 渐变色。
     */
    vg_gradient_t *gradient;
};

/**
 * @method vg_paint_create
 * 创建paint对象。
 *
 * @return {vg_path_t*} 返回paint对象。
 */
vg_paint_t *vg_paint_create(void);

/**
 * @method vg_paint_set_type
 * 设置类型。
 * @param {vg_path_t*} paint paint对象。
 * @param {vg_paint_type_t} type 类型。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t vg_paint_set_type(vg_paint_t *paint, vg_paint_type_t type);

/**
 * @method vg_paint_set_join_type
 * 设置交点类型。
 * @param {vg_path_t*} paint paint对象。
 * @param {vg_join_type_t_t} join 交点类型。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t vg_paint_set_join_type(vg_paint_t *paint, vg_join_type_t join);

/**
 * @method vg_paint_set_cap_type
 * 设置顶点类型。
 * @param {vg_path_t*} paint paint对象。
 * @param {vg_cap_type_t_t} cap 交点类型。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t vg_paint_set_cap_type(vg_paint_t *paint, vg_cap_type_t cap);

/**
 * @method vg_paint_set_blend_mode
 * 设置合成模式。
 * @param {vg_path_t*} paint paint对象。
 * @param {vg_blend_mode_t_t} blend 合成模式。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t vg_paint_set_blend_mode(vg_paint_t *paint, vg_blend_mode_t blend);

/**
 * @method vg_paint_set_color
 * 设置颜色。
 * @param {vg_path_t*} paint paint对象。
 * @param {color_t} color 颜色。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t vg_paint_set_color(vg_paint_t *paint, color_t color);

/**
 * @method vg_paint_set_line_width
 * 设置线宽。
 * @param {vg_path_t*} paint paint对象。
 * @param {line_width_t} line_width 线宽。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t vg_paint_set_line_width(vg_paint_t *paint, float line_width);

/**
 * @method vg_paint_set_linear_gradient
 * 设置线性渐变。
 * @param {vg_path_t*} paint paint对象。
 * @param {float} sx 起始点x坐标。
 * @param {float} sy 起始点y坐标。
 * @param {float} ex 结束点x坐标。
 * @param {float} ey 结束点y坐标。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t vg_paint_set_linear_gradient(vg_paint_t *paint, float sx, float sy, float ex, float ey);

/**
 * @method vg_paint_set_radial_gradient
 * 设置放射渐变。
 * @param {vg_path_t*} paint paint对象。
 * @param {float} sx 起始点x坐标。
 * @param {float} sy 起始点y坐标。
 * @param {float} ex 结束点x坐标。
 * @param {float} ey 结束点y坐标。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t vg_paint_set_radial_gradient(vg_paint_t *paint, float sx, float sy, float ex, float ey);

/**
 * @method vg_paint_add_gradient_stop
 * 增加渐变关键点。
 * @param {vg_path_t*} paint paint对象。
 * @param {color_t} color 颜色。
 * @param {float} stop 位置。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t vg_paint_add_gradient_stop(vg_paint_t *paint, color_t color, float stop);

/**
 * @method vg_paint_destroy
 * 销毁paint对象。
 * @param {vg_path_t*} paint paint对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t vg_paint_destroy(vg_paint_t *paint);

END_C_DECLS

#endif /*TK_VG_PAINT_H*/
