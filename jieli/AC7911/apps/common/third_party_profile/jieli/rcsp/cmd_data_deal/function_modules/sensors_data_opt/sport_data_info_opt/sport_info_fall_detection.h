#ifndef __RCSP_SPORT_FALL_DETECTION_H__
#define __RCSP_SPORT_FALL_DETECTION_H__

#include "typedef.h"
#include "app_config.h"

typedef struct fall_detection_mode {
    u8 phone_len;
    u8 phone_num[20];
} fall_detection_t;

int sport_fall_detection_get(fall_detection_t *fall_detect);

#endif
