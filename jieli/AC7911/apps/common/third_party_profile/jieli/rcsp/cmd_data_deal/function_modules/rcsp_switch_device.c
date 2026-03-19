#include "rcsp_config.h"
#include "rcsp_feature.h"
#include "btstack/avctp_user.h"
#include "rcsp_event.h"
#include "rcsp_manage.h"
#include "rcsp_bt_manage.h"
#include "rcsp_update.h"

/* #include "app_main.h" *////RCSP TODO

#if (RCSP_MODE)

/* #define RCSP_DEBUG_EN */

#ifdef  RCSP_DEBUG_EN
#define rcsp_putchar(x)                putchar(x)
#define rcsp_printf                    printf
#define rcsp_printf_buf(x,len)         printf_buf(x,len)
#else
#define rcsp_putchar(...)
#define rcsp_printf(...)
#define rcsp_printf_buf(...)
#endif

static u8 g_new_reconn_flag = 0;

static void rcsp_switch_device(u8 *data)
{
#if RCSP_UPDATE_EN
    if (get_jl_update_flag()) {
        if (RCSP_BLE == bt_3th_get_cur_bt_channel_sel()) {
            rcsp_printf("BLE_ CON START DISCON\n");
            JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_DEV_DISCONNECT, NULL, 0);
        } else {
            rcsp_printf("WAIT_FOR_SPP_DISCON\n");
        }
    } else {
        JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_SWITCH_DEVICE, &data[0], 1);
    }
#else
    JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_SWITCH_DEVICE, &data[0], 1);
#endif
}

u8 get_rcsp_support_new_reconn_flag(void)
{
    return g_new_reconn_flag;
}

void switch_device(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return ;
    }
    rcspModel->trans_chl = data[0];//指spp还是ble
    printf("rcspModel->trans_chl:%x\n", rcspModel->trans_chl);
    if (len > 1) {
        g_new_reconn_flag = data[1];
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, &g_new_reconn_flag, 1);
    } else {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0);
    }
    rcsp_switch_device(data);
}

#endif//RCSP_MODE


