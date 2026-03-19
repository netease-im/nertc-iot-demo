#include "awtk.h"
#include "mvvm/mvvm.h"
#include "hmi/hmi_service.h"
#include "hmi/log_message.h"
#include "hmi/history_data.h"
#include "csv/csv_file_object.h"

#ifndef APP_SYSTEM_BAR
#define APP_SYSTEM_BAR "system_bar"
#endif /*APP_SYSTEM_BAR*/

#ifndef APP_START_PAGE
#define APP_START_PAGE "home_page"
#endif /*APP_START_PAGE*/

/**
 * 注册自定义控件
 */
static ret_t custom_widgets_register(void)
{
    return RET_OK;
}

ret_t test_log_message(void)
{
    uint32_t i = 0;
    uint32_t n = 10000;
    char message[256] = {0};
    tk_object_t *model = hmi_service_get_default_model();
    navigator_request_t *req = navigator_request_create("log_message", NULL);
    view_model_t *vm = view_model_factory_create_model("log_message", req);
    csv_file_object_t *csv = CSV_FILE_OBJECT(VIEW_MODEL_ARRAY_OBJECT_WRAPPPER(vm)->obj);
    uint32_t max_rows = csv->csv->max_rows;

    ENSURE(model != NULL);
    ENSURE(vm != NULL);
    ENSURE(csv != NULL);
    ENSURE(vm != NULL);

    ENSURE(model != NULL);
    ENSURE(vm != NULL);
    ENSURE(csv != NULL);
    ENSURE(vm != NULL);
    view_model_exec(vm, TK_OBJECT_CMD_CLEAR, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == 0);

    for (i = 0; i < n; i++) {
        tk_snprintf(message, sizeof(message) - 1, "%d|17020%05d|大门|长时间没有关闭", i % 3, i);
        tk_object_set_prop_str(model, LOG_MESSAGE_PROP_NAME, message);
        if (i < max_rows) {
            ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == i + 1);
        } else {
            ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows);
        }
    }

    view_model_exec(vm, TK_OBJECT_CMD_SAVE, NULL);

    view_model_exec(vm, TK_OBJECT_CMD_CLEAR, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == 0);

    view_model_exec(vm, TK_OBJECT_CMD_RELOAD, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows);
    /*测试删除*/
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE, "0") == TRUE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_CLEAR, NULL) == TRUE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE_CHECKED, NULL) == FALSE);
    for (i = 0; i < max_rows; i++) {
        ENSURE(tk_object_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE, "0") == RET_OK);
        ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows - i - 1);
    }
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE, "0") == FALSE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_CLEAR, NULL) == FALSE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE_CHECKED, NULL) == FALSE);

    view_model_exec(vm, TK_OBJECT_CMD_RELOAD, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows);

    /*测试查询*/
    tk_object_set_prop_str(TK_OBJECT(vm), LOG_MESSAGE_PROP_QUERY_LEVEL, "0");
    tk_object_set_prop_str(TK_OBJECT(vm), LOG_MESSAGE_PROP_QUERY_END_DATE, "4000-01-01");

    tk_object_exec(TK_OBJECT(vm), CSV_CMD_QUERY, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows / 3);

    tk_object_exec(TK_OBJECT(vm), CSV_CMD_QUERY, "clear");
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows);

    tk_object_set_prop_str(TK_OBJECT(vm), LOG_MESSAGE_PROP_QUERY_LEVEL, "-1");
    tk_object_set_prop_str(TK_OBJECT(vm), LOG_MESSAGE_PROP_QUERY_KEYWORDS, "关闭");
    tk_object_exec(TK_OBJECT(vm), CSV_CMD_QUERY, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows);

    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE, "0") == TRUE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_CLEAR, NULL) == TRUE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE_CHECKED, NULL) == FALSE);
    for (i = 0; i < max_rows; i++) {
        ENSURE(tk_object_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE, "0") == RET_OK);
        ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows - i - 1);
    }
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE, "0") == FALSE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_CLEAR, NULL) == FALSE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE_CHECKED, NULL) == FALSE);

    /*测试clear查询结果*/
    view_model_exec(vm, TK_OBJECT_CMD_RELOAD, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows);

    /*测试查询*/
    tk_object_set_prop_str(TK_OBJECT(vm), LOG_MESSAGE_PROP_QUERY_LEVEL, "0");
    tk_object_set_prop_str(TK_OBJECT(vm), LOG_MESSAGE_PROP_QUERY_END_DATE, "4000-01-01");

    tk_object_exec(TK_OBJECT(vm), CSV_CMD_QUERY, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows / 3);

    tk_object_exec(TK_OBJECT(vm), TK_OBJECT_CMD_CLEAR, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == 0);

    tk_object_exec(TK_OBJECT(vm), CSV_CMD_QUERY, "clear");
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows * 2 / 3);

    TK_OBJECT_UNREF(vm);
    TK_OBJECT_UNREF(req);

    return RET_OK;
}

ret_t test_user_manager(void)
{
    uint32_t i = 0;
    uint32_t n = 10000;
    char message[256] = {0};
    tk_object_t *model = hmi_service_get_default_model();
    navigator_request_t *req = navigator_request_create("user_manager", NULL);
    view_model_t *vm = view_model_factory_create_model("user_manager", req);
    csv_file_object_t *csv = CSV_FILE_OBJECT(VIEW_MODEL_ARRAY_OBJECT_WRAPPPER(vm)->obj);
    uint32_t max_rows = csv->csv->max_rows;

    ENSURE(model != NULL);
    ENSURE(vm != NULL);
    ENSURE(csv != NULL);
    ENSURE(vm != NULL);

    ENSURE(model != NULL);
    ENSURE(vm != NULL);
    ENSURE(csv != NULL);
    ENSURE(vm != NULL);
    view_model_exec(vm, TK_OBJECT_CMD_CLEAR, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == 0);

    for (i = 0; i < n; i++) {
        view_model_t *uvm = NULL;
        tk_snprintf(message, sizeof(message) - 1, "user_manager(new=u%d|%d|03ac674216f3e15c761ee1a5e255f067953623c8b388b4459e13f978d7c846f4|%d|)",
                    i, i % 3, i);
        uvm = view_model_factory_create_model(message, req);
        tk_object_exec(TK_OBJECT(uvm), TK_OBJECT_CMD_ADD, NULL);

        if (i < max_rows) {
            ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == i + 1);
        } else {
            ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows);
        }
    }

    view_model_exec(vm, TK_OBJECT_CMD_SAVE, NULL);

    view_model_exec(vm, TK_OBJECT_CMD_CLEAR, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == 0);

    view_model_exec(vm, TK_OBJECT_CMD_RELOAD, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows);
    /*测试删除*/
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE, "0") == TRUE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_CLEAR, NULL) == TRUE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE_CHECKED, NULL) == FALSE);
    for (i = 0; i < max_rows; i++) {
        ENSURE(tk_object_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE, "0") == RET_OK);
        ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows - i - 1);
    }
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE, "0") == FALSE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_CLEAR, NULL) == FALSE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE_CHECKED, NULL) == FALSE);

    view_model_exec(vm, TK_OBJECT_CMD_RELOAD, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows);

    /*测试查询*/
    tk_object_set_prop_str(TK_OBJECT(vm), CSV_QUERY_PREFIX "name", "u100");

    tk_object_exec(TK_OBJECT(vm), CSV_CMD_QUERY, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == 0);

    tk_object_set_prop_str(TK_OBJECT(vm), CSV_QUERY_PREFIX "name", "u9900");

    tk_object_exec(TK_OBJECT(vm), CSV_CMD_QUERY, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == 1);

    tk_object_exec(TK_OBJECT(vm), CSV_CMD_QUERY, "clear");
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows);

    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE, "0") == TRUE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_CLEAR, NULL) == TRUE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE_CHECKED, NULL) == FALSE);
    for (i = 0; i < max_rows; i++) {
        ENSURE(tk_object_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE, "0") == RET_OK);
        ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows - i - 1);
    }
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE, "0") == FALSE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_CLEAR, NULL) == FALSE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE_CHECKED, NULL) == FALSE);

    /*测试clear查询结果*/
    view_model_exec(vm, TK_OBJECT_CMD_RELOAD, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows);

    /*测试查询*/
    tk_object_exec(TK_OBJECT(vm), TK_OBJECT_CMD_CLEAR, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == 0);

    TK_OBJECT_UNREF(vm);
    TK_OBJECT_UNREF(req);

    return RET_OK;
}

ret_t test_history_data(void)
{
    uint32_t i = 0;
    uint32_t n = 10000;
    char message[256] = {0};
    tk_object_t *model = hmi_service_get_default_model();
    navigator_request_t *req = navigator_request_create("history_data", NULL);
    view_model_t *vm = view_model_factory_create_model("history_data", req);
    csv_file_object_t *csv = CSV_FILE_OBJECT(VIEW_MODEL_ARRAY_OBJECT_WRAPPPER(vm)->obj);
    uint32_t max_rows = csv->csv->max_rows;

    ENSURE(model != NULL);
    ENSURE(vm != NULL);
    ENSURE(csv != NULL);
    ENSURE(vm != NULL);

    ENSURE(model != NULL);
    ENSURE(vm != NULL);
    ENSURE(csv != NULL);
    ENSURE(vm != NULL);
    view_model_exec(vm, TK_OBJECT_CMD_CLEAR, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == 0);

    for (i = 0; i < n; i++) {
        tk_snprintf(message, sizeof(message) - 1, "170203%04d|%d.1|%d.2|%d.3|%d.4", i, i % 10, i % 10, i % 10, i % 10);
        tk_object_set_prop_str(model, HISTORY_DATA_PROP_NAME, message);
        if (i < max_rows) {
            ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == i + 1);
        } else {
            ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows);
        }
    }

    view_model_exec(vm, TK_OBJECT_CMD_SAVE, NULL);

    view_model_exec(vm, TK_OBJECT_CMD_CLEAR, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == 0);

    view_model_exec(vm, TK_OBJECT_CMD_RELOAD, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows);
    /*测试删除*/
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE, "0") == TRUE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_CLEAR, NULL) == TRUE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE_CHECKED, NULL) == FALSE);
    for (i = 0; i < max_rows; i++) {
        ENSURE(tk_object_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE, "0") == RET_OK);
        ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows - i - 1);
    }
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE, "0") == FALSE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_CLEAR, NULL) == FALSE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE_CHECKED, NULL) == FALSE);

    view_model_exec(vm, TK_OBJECT_CMD_RELOAD, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows);

    /*测试查询*/
    tk_object_set_prop_str(TK_OBJECT(vm), "query.一氧化碳.min", "0");
    tk_object_set_prop_str(TK_OBJECT(vm), "query.一氧化碳.max", "1");

    tk_object_exec(TK_OBJECT(vm), CSV_CMD_QUERY, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows / 10);

    tk_object_exec(TK_OBJECT(vm), CSV_CMD_QUERY, "clear");
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows);

    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE, "0") == TRUE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_CLEAR, NULL) == TRUE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE_CHECKED, NULL) == FALSE);
    for (i = 0; i < max_rows; i++) {
        ENSURE(tk_object_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE, "0") == RET_OK);
        ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows - i - 1);
    }
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE, "0") == FALSE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_CLEAR, NULL) == FALSE);
    ENSURE(tk_object_can_exec(TK_OBJECT(vm), TK_OBJECT_CMD_REMOVE_CHECKED, NULL) == FALSE);

    /*测试clear查询结果*/
    view_model_exec(vm, TK_OBJECT_CMD_RELOAD, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == max_rows);

    /*测试查询*/
    tk_object_exec(TK_OBJECT(vm), TK_OBJECT_CMD_CLEAR, NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == 0);

    TK_OBJECT_UNREF(vm);
    TK_OBJECT_UNREF(req);
    return RET_OK;
}

#include "repository/view_model_repository.h"
#include "repository/repository_mvvm_const.h"

ret_t test_hmi_db(void)
{
    uint32_t i = 0;
    uint32_t n = 100;
    const char *name = NULL;
    tk_object_t *model = hmi_service_get_default_model();
    navigator_request_t *req = navigator_request_create("hmi_db", NULL);
    view_model_t *vm = view_model_factory_create_model("database(table=scores, key=name, start=0, filter='Chinese > 80', orderby=Chinese)", req);
    n = tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0);

    ENSURE(model != NULL);
    ENSURE(vm != NULL);
    ENSURE(n > 0);

    name = tk_object_get_prop_str(TK_OBJECT(vm), "items.[0].name");
    ENSURE(name != NULL);
    ENSURE(n > 0);

    n = 100;
    tk_object_set_prop_pointer(req->args, STR_ARG_REPOSITORY, VIEW_MODEL_REPOSITORY(vm)->r);
    for (i = 0; i < n; i++) {
        char record[256] = {0};
        view_model_t *uvm = NULL;
        tk_snprintf(record, sizeof(record) - 1, "record(name=u%d, Chinese=999, Math=%d, English=%d, memo='add')",
                    i, 80 + i % 20, 90 + i % 10);
        uvm = view_model_factory_create_model(record, req);
        ENSURE(uvm != NULL);
        tk_object_exec(TK_OBJECT(uvm), TK_OBJECT_CMD_ADD, NULL);
    }

    tk_object_exec(TK_OBJECT(vm), TK_OBJECT_CMD_SAVE, NULL);
    tk_object_set_prop_str(TK_OBJECT(vm), "filter", "Chinese=999");
    tk_object_exec(TK_OBJECT(vm), "search", NULL);
    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == 100);

    for (i = n; i > 0; i--) {
        char args[256] = {0};
        tk_snprintf(args, sizeof(args) - 1, "%d", i - 1);
        tk_object_exec(TK_OBJECT(vm), "remove", args);
    }

    ENSURE(tk_object_get_prop_int(TK_OBJECT(vm), TK_OBJECT_PROP_SIZE, 0) == 0);

    TK_OBJECT_UNREF(vm);
    TK_OBJECT_UNREF(req);
    return RET_OK;
}

ret_t application_init(void)
{
    custom_widgets_register();
    hmi_service_init(HMI_SERVICE_DEFAULT_UART);

    test_log_message();
    test_history_data();
    test_user_manager();
    test_hmi_db();

    tk_quit();

    return RET_OK;
}

/**
 * 退出程序
 */
ret_t application_exit(void)
{
    hmi_service_deinit();

    return RET_OK;
}
