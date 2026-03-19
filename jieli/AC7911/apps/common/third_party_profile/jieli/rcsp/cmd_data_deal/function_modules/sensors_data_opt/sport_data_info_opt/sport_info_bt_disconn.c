#include "rcsp_config.h"
#include "sport_info_opt.h"
#include "rcsp_event.h"
#include "rcsp_manage.h"

#include "sport_info_bt_disconn.h"

#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT)

void sport_info_bt_disconn_attr_set(void *priv, u8 attr, u8 *data, u16 len)
{
    u8 bt_reminder_switch = data[0];
    u8 bt_reminder_mode = data[1];
    sport_info_switch_record_update(SPORT_INFO_SWTICH_TYPE_BT_DISCONN, bt_reminder_switch, 1);
    sport_info_mode_record_update(SPORT_INFO_MODE_TYPE_BT_DISCONN, bt_reminder_mode);
}

u32 sport_info_bt_disconn_attr_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;

    u8 bt_reminder_data[2] = {0};
    bt_reminder_data[0] = !!sport_info_swtich_record_get(SPORT_INFO_SWTICH_TYPE_BT_DISCONN);

    u8 *mode_data = NULL;
    u16 mode_len = sport_info_record_get(SPORT_INFO_MODE_TYPE_BT_DISCONN, &mode_data);
    if (mode_data && mode_len) {
        bt_reminder_data[1] = mode_data[0];
    }
    rlen = add_one_attr(buf, buf_size, offset, attr, bt_reminder_data, sizeof(bt_reminder_data));

    return rlen;
}

#endif
