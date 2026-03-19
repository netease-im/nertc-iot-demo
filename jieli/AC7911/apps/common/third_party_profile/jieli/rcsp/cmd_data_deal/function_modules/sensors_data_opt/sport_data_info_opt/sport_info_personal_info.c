#include "rcsp_config.h"
#include "sport_info_opt.h"
#include "rcsp_event.h"
#include "rcsp_manage.h"
#include "sport_info_personal_info.h"


#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT)
#include "sport_data/watch_common.h"

void sport_info_personal_info_attr_set(void *priv, u8 attr, u8 *data, u16 len)
{
    printf("%s", __func__);
    personal_information info = {0};
    memcpy(&info, data, sizeof(personal_information));
    info.height = ((u8 *)&info.height)[0] << 8 | ((u8 *)&info.height)[1];
    info.weight = ((u8 *)&info.weight)[0] << 8 | ((u8 *)&info.weight)[1];

    info.birth_y = ((u8 *)&info.birth_y)[0] << 8 | ((u8 *)&info.birth_y)[1];

    extern u8 watch_time_age(u16 year, u8 month, u8 day);
    u8 age = watch_time_age(info.birth_y, info.birth_m, info.birth_d);
    printf("age=%d", age);
    struct watch_algo __watch_algo;
    struct personal_info PI;
    PI.height = info.height;
    PI.weight = info.weight;
    PI.age = age;
    PI.gender = info.gender;
    watch_algo_handle_get(&__watch_algo);
    __watch_algo.personal_information_set(&PI);

    sport_info_write_vm(VM_SPORT_INFO_PERSONAL_INFO_FLAG, (u8 *)&info, sizeof(personal_information));
}

u32 sport_info_personal_info_attr_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    printf("%s", __func__);
    u32 rlen = 0;
    personal_information info = {0};
    if (sport_personal_info_get(&info)) {
        info.height = ((u8 *)&info.height)[0] << 8 | ((u8 *)&info.height)[1];
        info.weight = ((u8 *)&info.weight)[0] << 8 | ((u8 *)&info.weight)[1];
        info.birth_y = ((u8 *)&info.birth_y)[0] << 8 | ((u8 *)&info.birth_y)[1];
    }
    rlen = add_one_attr(buf, buf_size, offset, attr, (u8 *)&info, sizeof(personal_information));
    return rlen;
}

#endif

