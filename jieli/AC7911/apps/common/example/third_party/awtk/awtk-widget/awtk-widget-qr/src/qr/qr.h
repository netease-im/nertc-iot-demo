/**
 * File:   qr.h
 * Author: AWTK Develop Team
 * Brief:  显示二维码的控件
 *
 * Copyright (c) 2020 - 2020 Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2020-06-01 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_QR_H
#define TK_QR_H
#include "qrencode.h"
#include "base/widget.h"

BEGIN_C_DECLS
/**
 * @class qr_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * 显示二维码的控件
 * qr\_t是[widget\_t](widget_t.md)的子类控件，widget\_t的函数均适用于qr\_t控件。
 *
 * 在xml中使用"qr"标签创建数值文本控件。如：
 *
 * ```xml
 * <!-- ui -->
 * <qr x="c" y="m" w="240" h="240" value="https://github.com/zlgopen/awtk"/>
 * ```
 * 在c代码中使用函数qr\_create创建数值文本控件。如：
 *
 * ```c
 *  widget_t* qr = qr_create(win, 10, 10, 128, 128);
 * ```
 * 可用通过style来设置控件的显示风格，如字体的大小和颜色等等。如：
 *
 * ```xml
 * <!-- style -->
 * <qr>
 *   <style name="default" >
 *     <normal fg_color="black" bg_color="white"/>
 *     <disable fg_color="gray" bg_color="#f0f0f0"/>
 *   </style>
 * </qr>
 * ```
 */
typedef struct _qr_t {
    widget_t widget;

    /**
     * @property {char*} value
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 要转成二维码的字符串。值。
     */
    char *value;

    /*private*/
    QRcode *qrcode;
} qr_t;

/**
 * @method qr_create
 * @annotation ["constructor", "scriptable"]
 * 创建qr对象
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} qr对象。
 */
widget_t *qr_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method qr_cast
 * 转换为qr对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget qr对象。
 *
 * @return {widget_t*} qr对象。
 */
widget_t *qr_cast(widget_t *widget);

/**
 * @method qr_set_value
 * 设置要转成二维码的字符串。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {const char*} value 格式字符串。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t qr_set_value(widget_t *widget, const char *value);

#define WIDGET_TYPE_QR "qr"
#define QR(widget) ((qr_t*)(qr_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(qr);

END_C_DECLS

#endif /*TK_QR_H*/
