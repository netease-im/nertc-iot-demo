#include "awtk.h"
#include "hmi/hmi_service.h"

#ifndef APP_SYSTEM_BAR
#define APP_SYSTEM_BAR ""
#endif /*APP_SYSTEM_BAR*/

#ifndef APP_START_PAGE
#define APP_START_PAGE "login"
#endif /*APP_START_PAGE*/

/**
 * 注册自定义控件
 */
static ret_t custom_widgets_register(void)
{
    return RET_OK;
}

/**
 * 初始化程序
 */
ret_t application_init(void)
{
    custom_widgets_register();
    hmi_service_init(HMI_SERVICE_DEFAULT_UART);

    if (strlen(APP_SYSTEM_BAR) > 0) {
        navigator_to(APP_SYSTEM_BAR);
    }

    return navigator_to(APP_START_PAGE);
}

/**
 * 退出程序
 */
ret_t application_exit(void)
{
    hmi_service_deinit();

    return RET_OK;
}
