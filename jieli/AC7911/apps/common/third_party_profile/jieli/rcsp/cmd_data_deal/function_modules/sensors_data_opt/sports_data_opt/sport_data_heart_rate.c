#include "rcsp_config.h"
#include "sport_data_heart_rate.h"
#include "sport_data_func.h"
#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT)
#include "sport_data/watch_common.h"

enum {
    SPORT_DATA_REAL_TIME_HEART_RATE,
    SPORT_DATA_RESTING_HEART_RATE,
    SPORT_DATA_MAX_HEART_RATE,
    SPORT_DATA_MAX,
};

//*----------------------------------------------------------------------------*/
/**@brief    心率获取函数
   @param	 priv - rcsp结构体
             attr - 属性值(对应0xA0协议的MASK中表示心率的那个BIT位，例如心率在MASK中对应的为是0，这个值就对应是0)
             sub_mask - 对应协议的SUB_MASK
             buf - 回复命令使用的buffer，把获取的数据按照回复包的格式填充(使用add_one_attr即可)buf即可进行回复
			 buffer_size - 回复命令使用的buffer大小，获取到的数据大小大于这个值则需要分包
			 offset - 当前buf已使用的大小
   @return   0-错误，非0-正常
   @note
*/
/*----------------------------------------------------------------------------*/

// 测试长度
#define HEART_RATE_DATA_MAX_LEN		300
static u8 *heart_rate_data = NULL;

u32 sport_data_func_attr_heart_rate_get(void *priv, u8 attr, u8 sub_mask, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
    u16 data_len = 0;
    struct watch_heart_rate __heartrate_hd;
    watch_heart_rate_handle_get(&__heartrate_hd);

    printf("%s", __func__);
    if (NULL == heart_rate_data) {
        heart_rate_data = zalloc(HEART_RATE_DATA_MAX_LEN);
    }

    if (sub_mask & BIT(SPORT_DATA_REAL_TIME_HEART_RATE)) {
        heart_rate_data[data_len++] = __heartrate_hd.heart_rate_data_get(LAST_DATA, NULL);
#if TCFG_HR_SENSOR_ENABLE
        sys_timeout_add(NULL, read_heart_rate_task, 1000);
#endif
    }

    if (sub_mask & BIT(SPORT_DATA_RESTING_HEART_RATE)) {
        heart_rate_data[data_len++]	= __heartrate_hd.heart_rate_data_get(MIN_DATA, NULL);
    }

    if (sub_mask & BIT(SPORT_DATA_MAX_HEART_RATE)) {
        heart_rate_data[data_len++]	= __heartrate_hd.heart_rate_data_get(MAX_DATA, NULL);
    }

    if (0xff == sub_mask) {
        data_len = HEART_RATE_DATA_MAX_LEN;

        memset(heart_rate_data, 0, data_len);
        data_len = __heartrate_hd.heart_rate_day_buf_len_get(NULL);
        data_len = __heartrate_hd.heart_rate_day_buf_get(heart_rate_data, data_len, NULL);

    }

    rlen = add_one_attr_with_submask_huge(buf, buf_size, offset, attr, sub_mask, heart_rate_data, data_len);

    // 如果需要每次都free某一变量，就把概变量放到概函数最后一个参数中，并以二级指针的方式传入
    // 如果这个指针是NULL，或者直接传入NULL，都不会free
    sport_data_global_var_deal(priv, attr, sub_mask, heart_rate_data, data_len, rlen, (void *)&heart_rate_data);
    return rlen;
}

#endif
