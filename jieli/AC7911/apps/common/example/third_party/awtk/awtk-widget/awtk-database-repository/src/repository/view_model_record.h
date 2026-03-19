/**
 * File:   view_model_record.h
 * Author: AWTK Develop Team
 * Brief:  view_model_record
 *
 * Copyright (c) 2020 - 2020  Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2020-09-08 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_VIEW_MODEL_RECORD_H
#define TK_VIEW_MODEL_RECORD_H

#include "repository/repository.h"
#include "mvvm/base/view_model.h"

BEGIN_C_DECLS

struct _view_model_record_t;
typedef struct _view_model_record_t view_model_record_t;

/**
 * @class view_model_record_t
 * @parent view_model_t
 *
 * record的查看、编辑和增加。
 *
 * 支持的属性：
 *
 * 由数据库表本身的字段决定。
 *
 * 支持的命令：
 *
 * add 用于新增记录。
 * save 用于更新记录。
 *
 */
struct _view_model_record_t {
    view_model_t view_model;

    /*private*/
    object_t *o;
    repository_t *r;
    navigator_request_t *req;
};

/**
 * @method view_model_record_create
 * 创建view_model对象。
 *
 * @param {navigator_request_t*} req req对象。
 *
 * @return {view_model_t*} 返回view_model对象。
 */
view_model_t *view_model_record_create(navigator_request_t *req);

#define VIEW_MODEL_RECORD(view_model) ((view_model_record_t*)(view_model))

END_C_DECLS

#endif /*TK_VIEW_MODEL_RECORD_H*/
