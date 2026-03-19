/**
 * File:   custom_widgets.c
 * Author: AWTK Develop Team
 * Brief:  custom widgets
 *
 * Copyright (c) 2018 - 2018  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2018-12-11 Xu ChaoZe <xuchaoze@zlg.cn> created
 *
 */

#include "custom_widgets.h"
#include "hour_weather.h"
#include "gesture.h"
#include "slide_menu_v.h"

ret_t custom_widgets_init()
{
    widget_factory_register(widget_factory(), WIDGET_TYPE_HOUR_WEATHER, hour_weather_create);
    widget_factory_register(widget_factory(), WIDGET_TYPE_GESTURE, gesture_create);
    widget_factory_register(widget_factory(), WIDGET_TYPE_SLIDE_MENU_V, slide_menu_v_create);

    return RET_OK;
}
