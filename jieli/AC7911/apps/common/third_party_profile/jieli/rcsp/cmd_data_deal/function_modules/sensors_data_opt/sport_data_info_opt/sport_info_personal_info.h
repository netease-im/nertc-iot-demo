#ifndef __RCSP_SPORT_PERSONAL_INFO_H__
#define __RCSP_SPORT_PERSONAL_INFO_H__

#include "typedef.h"
#include "app_config.h"

#pragma pack(1)
typedef struct sport_info_personal_info {
    u16 height;
    u16 weight;

    u16 birth_y;
    u8  birth_m;
    u8  birth_d;

    u8  gender;
} personal_information;

#pragma pack()

int sport_personal_info_get(personal_information *info);

#endif
