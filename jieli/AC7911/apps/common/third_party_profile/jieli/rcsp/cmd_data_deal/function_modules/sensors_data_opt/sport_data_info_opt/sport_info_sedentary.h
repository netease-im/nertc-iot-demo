#ifndef __RCSP_SPORT_INFO_SEDENTARY_H__
#define __RCSP_SPORT_INFO_SEDENTARY_H__

#include "typedef.h"
#include "app_config.h"

typedef struct sedentary_mode {
    u8 nop_mode;
    u8 begin_time_hour;
    u8 begin_time_min;
    u8 end_time_hour;
    u8 end_time_min;
} sedentary_t;

int sport_sedentary_get(sedentary_t *sedentary);

#endif
