#ifndef __RCSP_SPORT_EXERCISE_HEART_RATE_H__
#define __RCSP_SPORT_EXERCISE_HEART_RATE_H__

#include "typedef.h"
#include "app_config.h"

typedef struct exercise_heart_rate_mode {
    u8 max_heart_rate;
    u8 heart_rate_type;
} e_heart_rate;

int sport_exercise_heart_rate_get(e_heart_rate *heart_rate);

#endif
