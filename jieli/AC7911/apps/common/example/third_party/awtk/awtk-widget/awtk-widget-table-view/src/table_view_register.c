/**
 * File:   table_view.c
 * Author: AWTK Develop Team
 * Brief:  表格视图。
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
 * 2020-07-15 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "table_view_register.h"
#include "base/widget_factory.h"
#include "table_row/table_row.h"
#include "table_view/table_view.h"
#include "table_header/table_header.h"
#include "table_client/table_client.h"

ret_t table_view_register(void)
{
    widget_factory_register(widget_factory(), WIDGET_TYPE_TABLE_ROW, table_row_create);
    widget_factory_register(widget_factory(), WIDGET_TYPE_TABLE_HEADER, table_header_create);
    widget_factory_register(widget_factory(), WIDGET_TYPE_TABLE_CLIENT, table_client_create);

    return widget_factory_register(widget_factory(), WIDGET_TYPE_TABLE_VIEW, table_view_create);
}

const char *table_view_supported_render_mode(void)
{
    return "OpenGL|AGGE-BGR565|AGGE-BGRA8888|AGGE-MONO";
}
