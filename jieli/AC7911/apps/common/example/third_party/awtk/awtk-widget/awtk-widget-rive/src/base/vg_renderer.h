/**
 * File: vg_renderer.h
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

#ifndef TK_VG_RENDERER_H
#define TK_VG_RENDERER_H

#include "tkc/color.h"
#include "base/vgcanvas.h"
#include "base/vg_path.h"
#include "base/vg_paint.h"

BEGIN_C_DECLS

struct _vg_renderer_t;
typedef struct _vg_renderer_t vg_renderer_t;

/**
 * @class vg_renderer_stop_t
 * 渐变关键点。
 */
struct _vg_renderer_t {
    /**
     * @property {vgcanvas_t*} vg
     * @annotation ["readable"]
     * vgcanvas
     */
    vgcanvas_t *vg;
};

/**
 * @method vg_renderer_create
 * 创建renderer对象。
 *
 * @return {vg_renderer_t*} 返回renderer对象。
 */
vg_renderer_t *vg_renderer_create(vgcanvas_t *vg);

/**
 * @method vg_renderer_save
 * 保存状态。
 *
 * @param {vg_renderer_t*} renderer renderer对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t vg_renderer_save(vg_renderer_t *renderer);

/**
 * @method vg_renderer_restore
 * 恢复状态。
 *
 * @param {vg_renderer_t*} renderer renderer对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t vg_renderer_restore(vg_renderer_t *renderer);

/**
 * @method vg_renderer_draw_path
 * 绘制路径。
 *
 * @param {vg_renderer_t*} renderer renderer对象。
 * @param {vg_path_t*} path path对象。
 * @param {vg_paint_t*} paint paint对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t vg_renderer_draw_path(vg_renderer_t *renderer, const vg_path_t *path,
                            const vg_paint_t *paint);

/**
 * @method vg_renderer_clip_path
 * 裁减路径。
 *
 * @param {vg_renderer_t*} renderer renderer对象。
 * @param {vg_path_t*} path path对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t vg_renderer_clip_path(vg_renderer_t *renderer, const vg_path_t *path);

/**
 * @method vg_renderer_transform
 * transform。
 *
 * @param {vg_renderer_t*} renderer renderer对象。
 * @param {float} a a。
 * @param {float} b b。
 * @param {float} c c。
 * @param {float} d d。
 * @param {float} e e。
 * @param {float} f f。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t vg_renderer_transform(vg_renderer_t *renderer, float a, float b, float c, float d, float e,
                            float f);

/**
 * @method vg_renderer_destroy
 * 销毁renderer对象。
 *
 * @param {vg_renderer_t*} renderer renderer对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t vg_renderer_destroy(vg_renderer_t *renderer);

END_C_DECLS

#endif /*TK_VG_RENDERER_H*/
