#ifndef __RCSP_SPORT_RAISE_WRIST_H__
#define __RCSP_SPORT_RAISE_WRIST_H__

#include "typedef.h"
#include "app_config.h"

typedef struct raise_wrist_mode {
    u8 begin_time_hour;
    u8 begin_time_min;
    u8 end_time_hour;
    u8 end_time_min;
} raise_wrist_t;

int sport_raise_wrist_get(raise_wrist_t *raise_wrist);

#endif
