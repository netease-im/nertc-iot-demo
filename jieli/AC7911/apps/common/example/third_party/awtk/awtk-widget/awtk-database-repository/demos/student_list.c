#include "awtk.h"

#include "app_database.h"
#include "repository/repository.h"
#include "slidable_row_register.h"
#include "table_view_register.h"
#include "table_row/table_row.h"
#include "table_client/table_client.h"

#define COL_NAME "name"
#define COL_MATH "Math"
#define COL_MEMO "memo"
#define COL_CHINESE "Chinese"
#define COL_ENGLISH "English"


static ret_t student_list_reload(widget_t *win, repository_t *repository)
{
    widget_t *client = widget_lookup_by_type(win, WIDGET_TYPE_TABLE_CLIENT, TRUE);
    repository_select(repository, NULL);
    table_client_set_rows(client, repository_get_cache_nr(repository));
    table_client_reload(client);

    return RET_OK;
}

static ret_t student_list_on_quit(void *ctx, event_t *e)
{
    tk_quit();

    return RET_OK;
}

static ret_t student_list_on_clear(void *ctx, event_t *e)
{
    repository_t *repository = (repository_t *)ctx;
    widget_t *win = widget_get_window(WIDGET(e->target));

    repository_clear(repository);
    student_list_reload(win, repository);

    return RET_OK;
}

static ret_t student_list_on_trans_begin(void *ctx, event_t *e)
{
    repository_t *repository = (repository_t *)ctx;
    widget_t *win = widget_get_window(WIDGET(e->target));

    repository_trans_begin(repository);
    student_list_reload(win, repository);

    return RET_OK;
}

static ret_t student_list_on_trans_commit(void *ctx, event_t *e)
{
    repository_t *repository = (repository_t *)ctx;
    widget_t *win = widget_get_window(WIDGET(e->target));

    repository_trans_commit(repository);
    student_list_reload(win, repository);

    return RET_OK;
}

static ret_t student_list_on_trans_rollback(void *ctx, event_t *e)
{
    repository_t *repository = (repository_t *)ctx;
    widget_t *win = widget_get_window(WIDGET(e->target));

    repository_trans_rollback(repository);
    student_list_reload(win, repository);

    return RET_OK;
}

static table_row_t *table_row_of(widget_t *child)
{
    widget_t *iter = child;
    while (iter != NULL && !tk_str_eq(widget_get_type(iter), WIDGET_TYPE_TABLE_ROW)) {
        iter = iter->parent;
    }

    return TABLE_ROW(iter);
}

static ret_t student_list_on_load_data(void *ctx, uint32_t index, widget_t *row)
{
    repository_t *repository = (repository_t *)ctx;
    object_t *o = repository_get_cache_object(repository, index);
    const char *name = object_get_prop_str(o, COL_NAME);
    int32_t chinese = object_get_prop_int(o, COL_CHINESE, 0);
    int math = object_get_prop_int(o, COL_MATH, 0);
    int english = object_get_prop_int(o, COL_ENGLISH, 0);
    const char *memo = object_get_prop_str(o, COL_MEMO);

    widget_set_value_int(widget_lookup(row, "index", TRUE), index);
    widget_set_text_utf8(widget_lookup(row, "name", TRUE), name);
    widget_set_value_int(widget_lookup(row, "chinese", TRUE), chinese);
    widget_set_value_int(widget_lookup(row, "math", TRUE), math);
    widget_set_value_int(widget_lookup(row, "english", TRUE), english);
    widget_set_text_utf8(widget_lookup(row, "memo", TRUE), memo);

    return RET_OK;
}

static ret_t student_list_on_memo_changed(void *ctx, event_t *e)
{
    str_t str;
    widget_t *target = WIDGET(e->target);
    repository_t *repository = (repository_t *)ctx;
    table_row_t *row = table_row_of(WIDGET(e->target));
    object_t *o = repository_get_cache_object(repository, row->index);

    str_init(&str, 0);
    str_from_wstr(&str, target->text.str);
    object_set_prop_str(o, COL_MEMO, str.str);
    repository_update(repository, o);
    str_reset(&str);

    return RET_OK;
}

static ret_t student_list_on_remove(void *ctx, event_t *e)
{
    repository_t *repository = (repository_t *)ctx;
    table_row_t *row = table_row_of(WIDGET(e->target));
    widget_t *win = widget_get_window(WIDGET(e->target));

    repository_remove_index(repository, row->index);
    student_list_reload(win, repository);

    return RET_OK;
}

static ret_t student_list_on_create_row(void *ctx, uint32_t index, widget_t *row)
{
    widget_child_on(row, "memo", EVT_VALUE_CHANGED, student_list_on_memo_changed, ctx);
    widget_child_on(row, "remove", EVT_CLICK, student_list_on_remove, ctx);

    return RET_OK;
}

/**
 * 初始化
 */
ret_t application_init(void)
{
    table_view_register();
    slidable_row_register();
    app_database_init("student_list", "students.db");
    widget_t *win = window_open("student_list");
    repository_t *repository = app_database_create_repository("scores", "name");
    widget_t *client = widget_lookup(win, "table_client", TRUE);

    widget_child_on(win, "clear", EVT_CLICK, student_list_on_clear, repository);
    widget_child_on(win, "trans_begin", EVT_CLICK, student_list_on_trans_begin, repository);
    widget_child_on(win, "trans_commit", EVT_CLICK, student_list_on_trans_commit, repository);
    widget_child_on(win, "trans_rollback", EVT_CLICK, student_list_on_trans_rollback, repository);
    widget_child_on(win, "quit", EVT_CLICK, student_list_on_quit, NULL);

    table_client_set_on_load_data(client, student_list_on_load_data, repository);
    table_client_set_on_create_row(client, student_list_on_create_row, repository);
    repository_select(repository, NULL);
    table_client_set_rows(client, repository_get_cache_nr(repository));

    return RET_OK;
}

/**
 * 退出
 */
ret_t application_exit(void)
{
    app_database_deinit();
    return RET_OK;
}

#include "res/assets.inc"
#include "awtk_main.inc"
