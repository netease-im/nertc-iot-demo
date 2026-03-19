#include "system/includes.h"
#include "app_config.h"
#include "wifi/wifi_connect.h"
#include "event/key_event.h"
#ifdef CONFIG_DUER_LC_DEMO_ENABLE

extern void my_duer_websocket_client_thread_create(void);

void duer_test_dmeo()
{
    enum wifi_sta_connect_state state;
    while (1) {
        printf("Connecting to the network...\n");
        state = wifi_get_sta_connect_state();
        if (WIFI_STA_NETWORK_STACK_DHCP_SUCC == state) {
            printf("Network connection is successful!\n");
            break;
        }
        os_time_dly(500);
    }
    my_duer_websocket_client_thread_create();
}
/*
 *按键响应函数
 */
static int duer_key_event_handler(struct sys_event *e)
{
    int err;
    static int flag = 0;
    struct key_event *key = (struct key_event *)e->payload;

    printf("duer key value:%d ", key->value);

    switch (key->action) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
        case KEY_POWER:
            duer_test_dmeo();
            break;
        default:
            break;
        }
        break;
    case KEY_EVENT_LONG:
        break;
    default:
        break;
    }
    return false;
}

SYS_EVENT_STATIC_HANDLER_REGISTER(duer_key_event) = {
    .event_type     = SYS_KEY_EVENT,
    .prob_handler   = duer_key_event_handler,
    .post_handler   = NULL,
};
#endif
