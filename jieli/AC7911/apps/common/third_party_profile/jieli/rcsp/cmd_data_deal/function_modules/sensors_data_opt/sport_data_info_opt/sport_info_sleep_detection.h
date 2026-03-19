#ifndef __RCSP_SPORT_SLEEP_DETECTION_H__
#define __RCSP_SPORT_SLEEP_DETECTION_H__

#include "typedef.h"
#include "app_config.h"

typedef struct sleep_detection_mode {
    u8 begin_time_hour;
    u8 begin_time_min;
    u8 end_time_hour;
    u8 end_time_min;
} sleep_detection_t;


int sport_sleep_detection_get(sleep_detection_t *sleep_detection);

#endif
