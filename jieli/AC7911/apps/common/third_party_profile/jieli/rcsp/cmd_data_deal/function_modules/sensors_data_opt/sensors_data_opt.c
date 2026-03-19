#include "sensors_data_opt.h"
#include "sport_data_func.h"
#include "sport_info_sync.h"
#include "nfc_data_opt.h"
#include "sport_info_opt.h"

#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT)
int JL_rcsp_sensors_data_opt(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    if (0 == JL_rcsp_sports_data_funciton(priv, OpCode, OpCode_SN, data, len)) {
        return 0;
    }
    if (0 == JL_rcsp_nfc_data_funciton(priv, OpCode, OpCode_SN, data, len)) {
        return 0;
    }
    if (0 == JL_rcsp_sports_info_funciton(priv, OpCode, OpCode_SN, data, len)) {
        return 0;
    }
    if (0 == JL_rcsp_sports_info_sync_funciton(priv, OpCode, OpCode_SN, data, len)) {
        return 0;
    }
    return -1;
}
#else
int JL_rcsp_sensors_data_opt(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    return -1;
}
#endif
