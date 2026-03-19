/**
 * File:   table_header.c
 * Author: AWTK Develop Team
 * Brief:  table_header
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
 * 2020-07-18 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "table_header.h"
#include "base/widget_vtable.h"

const char *s_table_header_properties[] = {NULL};

TK_DECL_VTABLE(table_header) = {.size = sizeof(table_header_t),
                                .type = WIDGET_TYPE_TABLE_HEADER,
                                .clone_properties = s_table_header_properties,
                                .persistent_properties = s_table_header_properties,
                                .parent = TK_PARENT_VTABLE(widget),
                                .create = table_header_create
                               };

widget_t *table_header_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
    widget_t *widget = widget_create(parent, TK_REF_VTABLE(table_header), x, y, w, h);
    table_header_t *table_header = TABLE_HEADER(widget);
    return_value_if_fail(table_header != NULL, NULL);

    return widget;
}

widget_t *table_header_cast(widget_t *widget)
{
    return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, table_header), NULL);

    return widget;
}
