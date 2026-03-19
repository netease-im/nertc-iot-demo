/**
 * File:  table_client_custom_binder.c
 * Author: AWTK Develop Team
 * Brief: table_client_custom_binder
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
 * 2020-07-24 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "awtk.h"
#include "mvvm/mvvm.h"
#include "table_row/table_row.h"
#include "table_client/table_client.h"
#include "table_client_custom_binder.h"

static int32_t view_model_get_items_nr(object_t *obj)
{
    value_t v;
    if (object_is_collection(obj)) {
        if (object_get_prop(obj, VIEW_MODEL_PROP_ITEMS, &v) == RET_OK) {
            if (v.type == VALUE_TYPE_OBJECT) {
                return object_get_prop_int(value_object(&v), OBJECT_PROP_SIZE, 0);
            } else {
                return value_int(&v);
            }
        }
    }

    return object_get_prop_int(obj, OBJECT_PROP_SIZE, 0);
}

static ret_t table_client_on_load_data_mvvm(void *ctx, uint32_t row_index, widget_t *row)
{
    items_binding_t *binding = ITEMS_BINDING(ctx);
    widget_t *widget = widget_get_child(row->parent, 0);

    if (widget == row) {
        binding->start_item_index = TABLE_ROW(widget)->index;
        binding_context_update_to_view(BINDING_RULE_CONTEXT(binding));
    }

    return RET_OK;
}

static ret_t table_client_on_items_changed(void *ctx, event_t *e)
{
    value_t v;
    binding_rule_t *rule = BINDING_RULE(ctx);
    items_binding_t *binding = ITEMS_BINDING(rule);
    binding_context_t *bctx = BINDING_RULE_CONTEXT(rule);

    if (binding_context_get_prop_by_rule(bctx, rule->parent, binding->items_name, &v) == RET_OK) {
        if (v.type == VALUE_TYPE_OBJECT) {
            object_t *obj = value_object(&v);

            if (obj == OBJECT(e->target)) {
                widget_t *widget = WIDGET(BINDING_RULE_WIDGET(rule));
                table_client_set_rows(widget, view_model_get_items_nr(obj));
            }
        }
    }

    return RET_OK;
}

static ret_t table_client_on_before_paint(void *ctx, event_t *e)
{
    // 由于 binding_context_update_to_view 为 idle 执行，因此在 paint 之前触发 update，避免显示的数据不正确
    idle_dispatch();
    return RET_OK;
}

static ret_t table_client_on_prepare_row_mvvm(void *ctx, widget_t *client, uint32_t prepare_cnt)
{
    binding_rule_t *rule = BINDING_RULE(ctx);
    items_binding_t *binding = ITEMS_BINDING(rule);

    if (binding->fixed_widget_count != prepare_cnt) {
        value_t v;
        binding_context_t *bctx = BINDING_RULE_CONTEXT(rule);

        if (binding_context_get_prop_by_rule(bctx, rule->parent, binding->items_name, &v) == RET_OK) {
            if (v.type == VALUE_TYPE_OBJECT) {
                object_t *obj = value_object(&v);

                table_client_set_rows(client, view_model_get_items_nr(obj));

                binding->fixed_widget_count = prepare_cnt;
                binding_context_notify_items_changed_sync(bctx, obj);
            }
        }
    }

    return RET_OK;
}

static ret_t table_client_bind(binding_context_t *ctx, binding_rule_t *rule)
{
    if (binding_rule_is_items_binding(rule)) {
        emitter_t *emitter = EMITTER(ctx->view_model);
        items_binding_t *binding = ITEMS_BINDING(rule);
        widget_t *widget = BINDING_RULE_WIDGET(rule);
        return_value_if_fail(widget != NULL && ctx != NULL, RET_BAD_PARAMS);

        binding->fixed_widget_count = 0;
        table_client_set_on_prepare_row(widget, table_client_on_prepare_row_mvvm, rule);
        table_client_set_on_load_data(widget, table_client_on_load_data_mvvm, rule);
        emitter_on(emitter, EVT_ITEMS_CHANGED, table_client_on_items_changed, rule);
        widget_on(widget, EVT_BEFORE_PAINT, table_client_on_before_paint, NULL);
    }

    return RET_OK;
}

ret_t table_client_custom_binder_register(void)
{
    return custom_binder_register(WIDGET_TYPE_TABLE_CLIENT, table_client_bind);
}
