#include "rcsp.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "string.h"
#include "system/timer.h"
#include "app_core.h"
#include "spp_user.h"
/* #include "app_task.h" *////RCSP TODO
#include "system/task.h"
#include "rcsp_config.h"
#include "rcsp_event.h"
#include "btstack_3th_protocol_user.h"
#include "rcsp_manage.h"
#include "rcsp_setting_opt.h"


#if (RCSP_MODE)
#define RCSP_TASK_NAME   "rcsp"

#ifndef JL_RCSP_CUSTOM_APP_EN
#define RCSP_SPP_INTERACTIVE_SUPPORT	1
#define RCSP_BLE_INTERACTIVE_SUPPORT	1
#endif

struct RcspModel *__this = NULL;

extern void cmd_recieve(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len);
extern void cmd_recieve_no_respone(void *priv, u8 OpCode, u8 *data, u16 len);
extern void cmd_respone(void *priv, u8 OpCode, u8 status, u8 *data, u16 len);
extern void data_recieve(void *priv, u8 OpCode_SN, u8 CMD_OpCode, u8 *data, u16 len);
extern void data_recieve_no_respone(void *priv, u8 CMD_OpCode, u8 *data, u16 len);
extern void data_respone(void *priv, u8 status, u8 CMD_OpCode, u8 *data, u16 len);


extern void JL_recieve_packet_parse_process(void);
int JL_rcsp_recieve_deal(int param)
{
    JL_recieve_packet_parse_process();
    return 0;
}

void JL_rcsp_recieve_resume(void)
{
    int argv[3];
    argv[0] = (int)JL_rcsp_recieve_deal;
    argv[1] = 1;
    argv[2] = 0;
    int ret = os_taskq_post_type("app_core", Q_CALLBACK, 3, argv);
}

void JL_rcsp_resume_do(void)
{
    os_sem_post(&__this->sem);
    JL_rcsp_recieve_resume();
}

static void rcsp_process(void *p)
{
    int res;
    while (1) {
        os_sem_pend(&__this->sem, 0);
        JL_send_packet_process();
        /* JL_protocol_process(); */
    }
}


struct RcspModel *rcsp_handle_get(void)
{
    return __this;
}

static u16 rcsp_timer = 0;

static void rcsp_process_timer()
{
    os_sem_post(&__this->sem);
}

void rcsp_timer_contrl(u8 status)
{
    if (status) {
        if (rcsp_timer == 0) {
            rcsp_timer = sys_s_hi_timer_add(NULL, rcsp_process_timer, 2000);
        }
    } else {
        if (rcsp_timer) {
            sys_s_hi_timer_del(rcsp_timer);
            rcsp_timer = 0;
        }
    }
}

extern int rcsp_user_spp_state_specific(u8 packet_type);

static BT_3TH_USER_CB bt_rcsp_callback = {
    .type                             = APP_TYPE_RCSP,

    .bt_config                        = 0
#if (RCSP_BLE_INTERACTIVE_SUPPORT)
    | BT_CONFIG_BLE
#endif
#if (RCSP_SPP_INTERACTIVE_SUPPORT)
    | BT_CONFIG_SPP
#endif
    ,

    .bt_3th_handler.priv              = NULL,
    .bt_3th_handler.fw_ready          = NULL,
    .bt_3th_handler.fw_send           = NULL,
    .bt_3th_handler.CMD_resp          = cmd_recieve,
    .bt_3th_handler.CMD_no_resp       = cmd_recieve_no_respone,
    .bt_3th_handler.CMD_recieve_resp  = cmd_respone,
    .bt_3th_handler.DATA_resp         = data_recieve,
    .bt_3th_handler.DATA_no_resp      = data_recieve_no_respone,
    .bt_3th_handler.DATA_recieve_resp = data_respone,
    .bt_3th_handler.wait_resp_timeout = NULL,
    .BT_3TH_spp_state_specific        = rcsp_user_spp_state_specific,
    .BT_3TH_event_handler             = rcsp_user_event_handler,
};

void rcsp_init(void)
{
    if (__this) {
        return;
    }

    btstack_3th_protocol_user_init(&bt_rcsp_callback);

    //set_jl_mtu_resv();
    ///设置rcsp最大发送buf， 即MTU
    set_jl_mtu_send(272);
    //如果支持大文件传输可以通过修改接收buf大小优化传输速度
    set_jl_rcsp_recieve_buf_size(4 * 1024 + 272);

    u32 size = rcsp_protocol_need_buf_size();
    u8 *ptr = zalloc(size);
    ASSERT(ptr, "no ram for rcsp !!\n");
    JL_protocol_init(ptr, size);

    struct RcspModel *rcspModel = (struct RcspModel *)zalloc(sizeof(struct RcspModel));
    ASSERT(rcspModel, "no ram for rcsp !!\n");
    rcsp_config(rcspModel);
    __this = rcspModel;
    __this->rcsp_buf = ptr;

    os_sem_create(&__this->sem, 0);

    ///从vm获取相关配置
    rcsp_setting_init();

    int err = task_create(rcsp_process, (void *)rcspModel, RCSP_TASK_NAME);
    if (err) {
        printf("rcsp create fail %x\n", err);
    }
}

void rcsp_exit(void)
{
    //extern void rcsp_resume(void);
    //rcsp_resume();
    task_kill(RCSP_TASK_NAME);
    if (__this->rcsp_buf) {
        free(__this->rcsp_buf);
        __this->rcsp_buf = NULL;
    }
    if (__this) {
        free(__this);
        __this = NULL;
    }
    rcsp_opt_release();
}

#endif//RCSP_MODE
