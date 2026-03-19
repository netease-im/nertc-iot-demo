#include "rcsp_config.h"
#include "sport_info_sync.h"
#include "rcsp_event.h"
#include "rcsp_manage.h"
#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT)
#include "sport/sport_api.h"
#include "sport_data/watch_common.h"
#include "ui/ui_style.h"
static u8 sport_status;
enum {
    SPORT_INFO_SYNC_READ_INFO,
    SPORT_INFO_SYNC_STRTT_EXERCISE,
    SPORT_INFO_SYNC_END_EXERCISE_BY_APP,
    SPORT_INFO_SYNC_END_EXERCISE_BY_FW,
    SPORT_INFO_SYNC_PAUSE_EXERCISE,
    SPORT_INFO_SYNC_KEEP_EXERCISE,
    SPORT_INFO_SYNC_READ_REAL_TIME_INFO,
    SPORT_INFO_SYNC_REAL_TIME_INTERVAL_SET,
};
static struct watch_execise *__execise_hd = NULL;

extern int ui_sport_get_total_time(struct utime *t);

static void sport_ui_show(u32 ui_id)
{
    if (ui_id == ID_WINDOW_SPORT_INFO) {
        UI_WINDOW_PREEMPTION_POSH(ID_WINDOW_SPORT_INFO, NULL, NULL, UI_WINDOW_PREEMPTION_TYPE_SPORT);
    } else if (ui_id == ID_WINDOW_SPORT_RECORD) {
        UI_WINDOW_PREEMPTION_POSH(ID_WINDOW_SPORT_RECORD, NULL, NULL, UI_WINDOW_PREEMPTION_TYPE_SPORT);
    } else if (ui_id == ID_WINDOW_SPORT_CTRL) {
        UI_WINDOW_PREEMPTION_POSH(ID_WINDOW_SPORT_CTRL, NULL, NULL, UI_WINDOW_PREEMPTION_TYPE_SPORT);
    }
    if (ui_id != ID_WINDOW_SPORT_INFO) {
        UI_WINDOW_PREEMPTION_POP(ID_WINDOW_SPORT_INFO);
    }
    if (ui_id != ID_WINDOW_SPORT_RECORD) {
        UI_WINDOW_PREEMPTION_POP(ID_WINDOW_SPORT_RECORD);
    }
    if (ui_id != ID_WINDOW_SPORT_CTRL) {
        UI_WINDOW_PREEMPTION_POP(ID_WINDOW_SPORT_CTRL);
    }
    // reset auto shut down
    ui_auto_shut_down_disable();
    ui_auto_shut_down_enable();
}

static void sport_info_sync_read_info(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    if (__execise_hd == NULL) {
        __execise_hd = zalloc(sizeof(struct watch_execise));
        watch_execise_handle_get(__execise_hd);
    }
    int ret = 0;

    u8 resp_data[1 + 10] = {0};
    resp_data[0] = SPORT_INFO_SYNC_READ_INFO;

    u8 ctrl_status = __execise_hd->execise_ctrl_status_get();
    printf("ctrl_status=%d form  %s", ctrl_status, __func__);
    if (ctrl_status == 0) { // resp_data[1 + 0] = 运动模式，失败ret = 非0，直接goto __sport_info_sync_read_info_end
        resp_data[1] = 0x00;
    } else {
        resp_data[1] =  __execise_hd->execise_mode_get();
    }
    u8 e_status = ctrl_status; //resp_data[1 + 1] = 运动状态，失败同上

    if ((e_status == 2) | (e_status == 4)) {
        resp_data[1 + 1] = 0x00;
    } else if ((e_status == 1) | (e_status == 3)) {
        resp_data[1 + 1] = 0x01;
    }
    u32 e_start_time = get_sport_start_time(NULL);
    resp_data[1 + 2] = (e_start_time >> 24 & 0xff); // resp_data[1 + (2:5)] = 运动id，这里需要转化为大端，失败同上
    resp_data[1 + 3] = (e_start_time >> 16 & 0xff);
    resp_data[1 + 4] = (e_start_time >> 8  & 0xff);
    resp_data[1 + 5] = (e_start_time >> 0  & 0xff);
    resp_data[1 + 6] = 0x00; // resp_data[1 + 6] = 是否需要app记录gps，失败同上
    resp_data[1 + 7] = 0x00; // resp_data[1 + 7] = 最大心率，失败同上
    resp_data[1 + 8] = 0x03; // resp_data[1 + (8:9)] = app读取定时读取数据的间隔时间，这里需要转化为大端，失败同上
    resp_data[1 + 9] = 0xE8;


__sport_info_sync_read_info_end:
    if (ret) {
        // 失败
        printf("%s fail", __func__);
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_FAIL, OpCode_SN, NULL, 0);
    } else {
        // 成功
        printf("%s succ", __func__);
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, resp_data, sizeof(resp_data));
    }
}

// 当data是NULL时，就表示固件主动推送数据给app
int sport_info_sync_start_exercise_resp(u8 *data, u16 *data_len)
{
    if (__execise_hd == NULL) {
        __execise_hd = zalloc(sizeof(struct watch_execise));
        watch_execise_handle_get(__execise_hd);
    }
    int ret = 0;
    u8 resp_data[1 + 1] = {0};
    resp_data[0] = SPORT_INFO_SYNC_STRTT_EXERCISE;
    // resp_data[1] = 运动模式，如果失败ret = 非0，直接goto __sport_info_sync_sync_start_exercise_nodify_end

    u8 ctrl_status = __execise_hd->execise_ctrl_status_get();
    printf("ctrl_status=%d form  %s", ctrl_status, __func__);
    if (ctrl_status == 0) {
        resp_data[1] = 0x00;
    } else {
        resp_data[1] = __execise_hd->execise_mode_get();
    }
__sport_info_sync_sync_start_exercise_nodify_end:
    if (data) {
        memcpy(data, resp_data, sizeof(resp_data));
        *data_len = sizeof(resp_data);
    } else if (0 == ret) {
        // 固件 -> app
        JL_CMD_send(JL_OPCODE_SPORTS_DATA_SYNC, resp_data, sizeof(resp_data), JL_NOT_NEED_RESPOND);
    }

    return ret;

}

static void sport_info_sync_start_exercise(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    // app触发开始运动
    if (__execise_hd == NULL) {
        __execise_hd = zalloc(sizeof(struct watch_execise));
        watch_execise_handle_get(__execise_hd);
    }
    sport_status = __execise_hd->execise_ctrl_status_get();
    printf("sport_status=%d", sport_status);
    if (sport_status == 0) {
        __execise_hd->execise_ctrl_status_set(data[0], SPORT_STATUS_START);
        watch_sport_start(data[0]);
        printf("sport_type=%d", data[0]);
        sport_ui_show(ID_WINDOW_SPORT_INFO);
        ui_auto_goto_dial_disable();
    }

    int ret = sport_info_sync_start_exercise_resp(data, &len);
    if (ret) {
        // 失败
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_FAIL, OpCode_SN, NULL, 0);
    } else {
        // 成功
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, data, len);
    }

}

static void sport_info_sync_end_exercise_by_app(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    if (__execise_hd == NULL) {
        __execise_hd = zalloc(sizeof(struct watch_execise));
        watch_execise_handle_get(__execise_hd);
    }
    int ret = 0;
    u8 resp_data[1 + 32] = {0};
    sport_status = __execise_hd->execise_ctrl_status_get();
    printf("sport_status=%d", sport_status);

    if (sport_status != 0) {
        __execise_hd->execise_ctrl_status_set(-1, SPORT_STATUS_STOP);
        watch_sport_stop();
        __execise_hd->execise_ctrl_status_clr();
        sport_ui_show(ID_WINDOW_SPORT_RECORD);
        ui_auto_goto_dial_enable();
    }
    struct motion_info __execise_info;
    if (__execise_hd == NULL) {
        __execise_hd = zalloc(sizeof(struct watch_execise));
        watch_execise_handle_get(__execise_hd);
    }
    __execise_hd->execise_info_get(&__execise_info);
    resp_data[0] = SPORT_INFO_SYNC_END_EXERCISE_BY_APP;
    u32 e_end_time =	get_sport_end_time(NULL); // resp_data[1 + (0:3)] = 结束时间，这里需要转化成大端，失败ret = 非0，直接goto __sport_info_sync_end_exercise_by_app_end
    resp_data[1 + 0] = (e_end_time >> 24) & 0xff;
    resp_data[1 + 1] = (e_end_time >> 16) & 0xff;
    resp_data[1 + 2] = (e_end_time >> 8) & 0xff;
    resp_data[1 + 3] = (e_end_time) & 0xff;
    //	u32 e_recovery_time=get_recovery_time(NULL);// resp_data[1 + (4:7)] = 运动回复时间，这里需要转化为大端，失败同上
    resp_data[1 + 4] = 0;
    resp_data[1 + 5] = 1;
    resp_data[1 + 6] = 0;
    resp_data[1 + 7] = 10;
    u16 e_id = get_sport_recode_id();
    u16 e_size = get_sport_recode_size();
    resp_data[1 + 8] = (e_id >> 8) & 0xff;
    resp_data[1 + 9] = e_id & 0xff;
    resp_data[1 + 10] = (e_size >> 8) & 0xff;
    resp_data[1 + 11] = (e_size & 0xff);
    u32 e_intensity_time[5];
    memcpy(e_intensity_time, __execise_info.intensity_time, 20);
    for (int i = 0; i < 5; i++) {
        resp_data[1 + 12 + 4 * i] = (e_intensity_time[i] >> 24) & 0xff;
        resp_data[1 + 13 + 4 * i] = (e_intensity_time[i] >> 16) & 0xff;
        resp_data[1 + 14 + 4 * i] = (e_intensity_time[i] >> 8) & 0xff;
        resp_data[1 + 15 + 4 * i] = (e_intensity_time[i]) & 0xff;
    }

    // resp_data[1 + (8:31)] = 运动强度，需要转化为大端,失败同上
    __execise_hd->execise_ctrl_status_clr();
    if (__execise_hd != NULL) {
        free(__execise_hd);
        __execise_hd = NULL;
    }
__sport_info_sync_end_exercise_by_app_end:
    if (ret) {
        // 失败
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_FAIL, OpCode_SN, NULL, 0);
    } else {
        // 成功
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, resp_data, sizeof(resp_data));
    }
}

int sport_info_sync_end_exercise_by_fw(void)
{
    // 固件主动推送
    struct motion_info __execise_info;
    if (__execise_hd == NULL) {
        __execise_hd = zalloc(sizeof(struct watch_execise));
        watch_execise_handle_get(__execise_hd);
    }
    __execise_hd->execise_info_get(&__execise_info);
    int ret = 0;
    u8 resp_data[1 + 32] = {0};
    resp_data[0] = SPORT_INFO_SYNC_END_EXERCISE_BY_FW;
    u32 e_end_time =	get_sport_end_time(NULL);
    // resp_data[1 + (0:3)] = 结束时间，这里需要转化成大端，失败ret = 非0，直接goto __sport_info_sync_end_exercise_by_fw_end
    resp_data[1 + 0] = (e_end_time >> 24) & 0xff;
    resp_data[1 + 1] = (e_end_time >> 16) & 0xff;
    resp_data[1 + 2] = (e_end_time >> 8) & 0xff;
    resp_data[1 + 3] = (e_end_time) & 0xff;

    // resp_data[1 + (4:7)] = 运动回复时间，这里需要转化为大端，失败同上
    resp_data[1 + 4] = 0;
    resp_data[1 + 5] = 1;
    resp_data[1 + 6] = 0;
    resp_data[1 + 7] = 10;
    u16 e_id = get_sport_recode_id();
    u16 e_size = get_sport_recode_size();
    resp_data[1 + 8] = (e_id >> 8) & 0xff;
    resp_data[1 + 9] = e_id & 0xff;
    resp_data[1 + 10] = (e_size >> 8) & 0xff;
    resp_data[1 + 11] = (e_size & 0xff);
    // resp_data[1 + (8:31)] = 运动强度，需要转化为大端？失败同上
    u32 e_intensity_time[5];
    memcpy(e_intensity_time, __execise_info.intensity_time, 20);
    for (int i = 0; i < 5; i++) {
        resp_data[1 + 12 + 4 * i] = (e_intensity_time[i] >> 24) & 0xff;
        resp_data[1 + 13 + 4 * i] = (e_intensity_time[i] >> 16) & 0xff;
        resp_data[1 + 14 + 4 * i] = (e_intensity_time[i] >> 8) & 0xff;
        resp_data[1 + 15 + 4 * i] = (e_intensity_time[i]) & 0xff;
    }
    if (__execise_hd != NULL) {
        free(__execise_hd);
        __execise_hd = NULL;
    }
__sport_info_sync_end_exercise_by_fw_end:
    if (0 == ret) {
        JL_CMD_send(JL_OPCODE_SPORTS_DATA_SYNC, resp_data, sizeof(resp_data), JL_NOT_NEED_RESPOND);
    }
    return ret;
}


int sport_info_sync_pause_exercise_resp(void)
{
    // 固件主动推送暂停运动命令给app
    u8 resp_data = SPORT_INFO_SYNC_PAUSE_EXERCISE;
    JL_CMD_send(JL_OPCODE_SPORTS_DATA_SYNC, &resp_data, sizeof(resp_data), JL_NOT_NEED_RESPOND);
    return 0;

}

static void sport_info_sync_pause_exrcise(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    if (__execise_hd == NULL) {
        __execise_hd = zalloc(sizeof(struct watch_execise));
        watch_execise_handle_get(__execise_hd);
    }
    sport_status = __execise_hd->execise_ctrl_status_get();
    printf("sport_status=%d", sport_status);
    if ((sport_status == 1) | (sport_status == 3)) {
        __execise_hd->execise_ctrl_status_set(-1, SPORT_STATUS_PAUSE);
        watch_sport_pause();
        sport_ui_show(ID_WINDOW_SPORT_CTRL);
    }
    // 暂停运动
    u8 resp_data = SPORT_INFO_SYNC_PAUSE_EXERCISE;
    JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, &resp_data, 1);
}

int sport_info_sync_keep_exercise_resp(void)
{
    // 固件主动推送继续运动命令给app
    u8 resp_data = SPORT_INFO_SYNC_KEEP_EXERCISE;
    JL_CMD_send(JL_OPCODE_SPORTS_DATA_SYNC, &resp_data, sizeof(resp_data), JL_NOT_NEED_RESPOND);
    return 0;

}

static void sport_info_sync_keep_exercise(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{

    // 继续运动
    sport_status = __execise_hd->execise_ctrl_status_get();
    printf("sport_status=%d", sport_status);
    if (sport_status == 2) {
        __execise_hd->execise_ctrl_status_set(-1, SPORT_STATUS_CONTINNUE);
        watch_sport_restart();
        sport_ui_show(ID_WINDOW_SPORT_INFO);
    }
    u8 resp_data = SPORT_INFO_SYNC_KEEP_EXERCISE;
    JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, &resp_data, 1);
}

static void sport_info_sync_read_real_time_info(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    printf("%s", __func__);
    struct motion_info __execise_info;
    if (__execise_hd == NULL) {
        __execise_hd = zalloc(sizeof(struct watch_execise));
        watch_execise_handle_get(__execise_hd);
    }
    __execise_hd->execise_info_get(&__execise_info);

    int ret = 0;
    u8 resp_data[1 + 21] = {0};
    resp_data[0] = SPORT_INFO_SYNC_READ_REAL_TIME_INFO;
    resp_data[1 + 0] = 0x00; // resp_data[1 + 0] = 类型，失败ret = 非0，goto __sport_info_sync_read_real_time_info_end
    // resp_data[1 + (1:4)] = 运动步数，需要转化为大端，失败同上
    u32 e_step = __execise_info.step;
    resp_data[1 + 1] = (e_step >> 24) & 0xff;
    resp_data[1 + 2] = (e_step >> 16) & 0xff;
    resp_data[1 + 3] = (e_step >> 8) & 0xff;
    resp_data[1 + 4] = (e_step) & 0xff;
    // resp_data[1 + (5:6)] = 运动距离，需要转化为大端，失败同上
    u16 e_distance = __execise_info.step_distance;
    resp_data[1 + 5] = (e_distance >> 8);
    resp_data[1 + 6] = e_distance & 0xff;
    // resp_data[1 + (7:10)] = 运动时长，需要转化为大端，失败同上
    u32 e_motion_time = ui_sport_get_total_time(NULL); //跟UI同步时间
    resp_data[1 + 7] = (e_motion_time >> 24) & 0xff;
    resp_data[1 + 8] = (e_motion_time >> 16) & 0xff;
    resp_data[1 + 9] = (e_motion_time >> 8) & 0xff;
    resp_data[1 + 10] = (e_motion_time) & 0xff;
    // resp_data[1 + (11:12)] = 速度，需要转化为大端，失败同上
    u16 e_speed = __execise_info.motion_speed;
    resp_data[1 + 11] = e_speed >> 8;
    resp_data[1 + 12] = e_speed & 0xff;
    // resp_data[1 + (13:14)] = 热量，需要转化为大端，失败同上
    u16 e_kcal = __execise_info.caloric;
    resp_data[1 + 13] = e_kcal >> 8;
    resp_data[1 + 14] = e_kcal & 0xff;
    // resp_data[1 + (15:16)] = 步频，需要转化为大端，失败同上
    u16 e_step_freq = __execise_info.motion_step_freq;
    resp_data[1 + 15] = e_step_freq >> 8;
    resp_data[1 + 16] = e_step_freq & 0xff;
    // resp_data[1 + (17:18)] = 步幅，需要转化为大端，失败同上
    u16 e_step_stride = __execise_info.motion_step_stride;
    resp_data[1 + 17] = e_step_stride >> 8;
    resp_data[1 + 18] = e_step_stride & 0xff;
    // resp_data[1 + 19] = 运动强度区间，失败同上
    resp_data[1 + 19] = __execise_info.intensity;
    // resp_data[1 + 20] = 运动实时心率，失败同上
    struct watch_heart_rate __heartrate_hd;
    watch_heart_rate_handle_get(&__heartrate_hd);
    resp_data[1 + 20] = __heartrate_hd.heart_rate_data_get(LAST_DATA, NULL);
    printf("%x-0- %04x %02x %04x %04x %02x %02x %02x", resp_data[0], e_step, e_distance, e_motion_time, e_speed, e_kcal, e_step_freq, e_step_stride);
__sport_info_sync_read_real_time_info_end:
    if (ret) {
        // 失败
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_FAIL, OpCode_SN, NULL, 0);
    } else {
        // 成功
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, resp_data, sizeof(resp_data));
    }
}

static void JL_rcsp_sports_info_sync_real_time_interval_set(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    // 测试使用的命令，固件实时数据采集间隔时间
    u16 real_time_interval = data[0] << 8 | data[1];

    u8 resp_data = SPORT_INFO_SYNC_REAL_TIME_INTERVAL_SET;
    JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, &resp_data, 1);
}

int JL_rcsp_sports_info_sync_funciton(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)

{
    int ret = -1;
    if (JL_OPCODE_SPORTS_DATA_SYNC == OpCode) {
        u8 op = data[0];
        switch (op) {
        case SPORT_INFO_SYNC_READ_INFO:
            sport_info_sync_read_info(priv, OpCode, OpCode_SN, data + 1, len - 1);
            break;
        case SPORT_INFO_SYNC_STRTT_EXERCISE:
            sport_info_sync_start_exercise(priv, OpCode, OpCode_SN, data + 1, len - 1);
            break;
        case SPORT_INFO_SYNC_END_EXERCISE_BY_APP:
            sport_info_sync_end_exercise_by_app(priv, OpCode, OpCode_SN, data + 1, len - 1);
            break;
        case SPORT_INFO_SYNC_PAUSE_EXERCISE:
            sport_info_sync_pause_exrcise(priv, OpCode, OpCode_SN, data + 1, len - 1);
            break;
        case SPORT_INFO_SYNC_KEEP_EXERCISE:
            sport_info_sync_keep_exercise(priv, OpCode, OpCode_SN, data + 1, len - 1);
            break;
        case SPORT_INFO_SYNC_READ_REAL_TIME_INFO:
            sport_info_sync_read_real_time_info(priv, OpCode, OpCode_SN, data + 1, len - 1);
            break;
        case SPORT_INFO_SYNC_REAL_TIME_INTERVAL_SET:
            JL_rcsp_sports_info_sync_real_time_interval_set(priv, OpCode, OpCode_SN, data + 1, len - 1);
            break;
        }
        ret = 0;
    }
    return ret;
}
#else
int sport_info_sync_end_exercise_by_fw(void)
{
    return 0;
}
int sport_info_sync_start_exercise_resp(u8 *data, u16 *data_len)
{
    return 0;
}
int sport_info_sync_keep_exercise_resp(void)
{
    return 0;
}
int sport_info_sync_pause_exercise_resp(void)
{
    return 0;
}
#endif
