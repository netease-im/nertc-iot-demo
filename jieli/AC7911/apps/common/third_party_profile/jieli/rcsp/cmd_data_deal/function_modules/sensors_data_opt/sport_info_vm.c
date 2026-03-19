#include "rcsp_config.h"
#include "sport_info_opt.h"
#include "rcsp_event.h"
#include "rcsp_manage.h"

#include "sport_data_info_opt/sport_info_exercise_heart_rate.h"
#include "sport_data_info_opt/sport_info_fall_detection.h"
#include "sport_data_info_opt/sport_info_personal_info.h"
#include "sport_data_info_opt/sport_info_raise_wrist.h"
#include "sport_data_info_opt/sport_info_sedentary.h"
#include "sport_data_info_opt/sport_info_sleep_detection.h"


#if ((TCFG_HR_SENSOR_ENABLE ||TCFG_SPO2_SENSOR_ENABLE ||  TCFG_GSENSOR_ENABLE)||(RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT))

static u32 g_sport_info_switch_record = 0;
static u8 g_sport_info_mode_record[SPORT_INFO_MODE_TYPE_MAX + 1] = {0};
// 读vm
int sport_info_write_vm(int vm_id, u8 *data, u16 data_len)
{
    u8 *tmp_var = zalloc(data_len);
    if (NULL == tmp_var) {
        goto __sport_info_write_vm_end;
    }
    syscfg_read(vm_id, tmp_var, data_len);
    if (0 != memcmp(tmp_var, data, data_len)) {
        syscfg_write(vm_id, data, data_len);
    }

__sport_info_write_vm_end:
    if (tmp_var) {
        free(tmp_var);
    }
    return 0;
}

// 写vm
int sport_info_read_vm(int vm_id, u8 *data, u16 data_len)
{
    int ret = 0;
    u8 *tmp_var = zalloc(data_len);
    if (NULL == tmp_var) {
        printf("no enough ram\n");
        goto __sport_info_read_vm_end;
    }
    if (data_len == syscfg_read(vm_id, tmp_var, data_len)) {
        memcpy(data, tmp_var, data_len);
        ret = data_len;
    }

__sport_info_read_vm_end:
    if (tmp_var) {
        free(tmp_var);
    }
    return ret;
}

// 更新开关
void sport_info_switch_record_update(u8 switch_type, u8 switch_state, u8 write_vm)
{
    if (switch_state) {
        g_sport_info_switch_record |= BIT(switch_type);
    } else {
        g_sport_info_switch_record &= ~BIT(switch_type);
    }
    if (write_vm) {
        sport_info_write_vm(VM_SPORT_INFO_SWITCH_FLAG, (u8 *)&g_sport_info_switch_record, sizeof(g_sport_info_switch_record));
    }
}

// 获取开关信息
u32 sport_info_swtich_record_get(u8 switch_type)
{
    u32 sport_info_switch_record = 0;
    if (sport_info_read_vm(VM_SPORT_INFO_SWITCH_FLAG, (u8 *)&g_sport_info_switch_record, sizeof(g_sport_info_switch_record))) {
        sport_info_switch_record = g_sport_info_switch_record;
    }
    if (switch_type < SPORT_INFO_SWTICH_TYPE_MAX) {
        return (sport_info_switch_record & BIT(switch_type));
    }
    return sport_info_switch_record;
}

// 更新模式
void sport_info_mode_record_update(u8 mode_type, u8 mode)
{
    g_sport_info_mode_record[0] = sizeof(g_sport_info_mode_record) - 1;
    g_sport_info_mode_record[mode_type + 1] = mode;
    sport_info_write_vm(VM_SPORT_INFO_MODE_FLAG, g_sport_info_mode_record, sizeof(g_sport_info_mode_record));
}

// 获取模式信息
u16 sport_info_record_get(u8 mode_type, u8 *mode_data[])
{
    u16 data_len = sport_info_read_vm(VM_SPORT_INFO_MODE_FLAG, g_sport_info_mode_record, sizeof(g_sport_info_mode_record));
    *mode_data = g_sport_info_mode_record;
    if (mode_type < SPORT_INFO_MODE_TYPE_MAX) {
        *mode_data += mode_type + 1;
        data_len = 1;
    }
    return data_len;
}

int sport_exercise_heart_rate_get(e_heart_rate *heart_rate)
{
    return sport_info_read_vm(VM_SPORT_INFO_EXERCISE_HEART_RATE, (u8 *)heart_rate, sizeof(e_heart_rate));
}


int sport_fall_detection_get(fall_detection_t *fall_detect)
{
    return sport_info_read_vm(VM_SPORT_INFO_FALL_DETECTION, (u8 *)fall_detect, sizeof(fall_detection_t));

}

int sport_personal_info_get(personal_information *info)
{
    return sport_info_read_vm(VM_SPORT_INFO_PERSONAL_INFO_FLAG, (u8 *)info, sizeof(personal_information));
}

int sport_raise_wrist_get(raise_wrist_t *raise_wrist)
{
    return sport_info_read_vm(VM_SPORT_INFO_RAISE_WRIST, (u8 *)raise_wrist, sizeof(raise_wrist_t));
}


int sport_sedentary_get(sedentary_t *sedentary)
{
    return sport_info_read_vm(VM_SPORT_INFO_SEDENTARY, (u8 *)sedentary, sizeof(sedentary_t));
}

int sport_sleep_detection_get(sleep_detection_t *sleep_detection)
{
    return sport_info_read_vm(VM_SPORT_INFO_SLEEP_DETECTION, (u8 *)sleep_detection, sizeof(sleep_detection_t));
}

#endif
