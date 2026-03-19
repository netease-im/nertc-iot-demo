/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.io/license.html
 */

#ifndef _LLD_GMOUSE_MCU_BOARD_H
#define _LLD_GMOUSE_MCU_BOARD_H

// Resolution and Accuracy Settings
#define GMOUSE_MCU_PEN_CALIBRATE_ERROR      8
#define GMOUSE_MCU_PEN_CLICK_ERROR          6
#define GMOUSE_MCU_PEN_MOVE_ERROR           4
#define GMOUSE_MCU_FINGER_CALIBRATE_ERROR   14
#define GMOUSE_MCU_FINGER_CLICK_ERROR       18
#define GMOUSE_MCU_FINGER_MOVE_ERROR        14
#define GMOUSE_MCU_Z_MIN                    0
#define GMOUSE_MCU_Z_MAX                    1
#define GMOUSE_MCU_Z_TOUCHON                1
#define GMOUSE_MCU_Z_TOUCHOFF               0

// How much extra data to allocate at the end of the GMouse structure for the board's use
#define GMOUSE_MCU_BOARD_DATA_SIZE      0

static GMouse *gm;
void ugfx_touch_event_process(u16 x, u16 y, u8 event_press)
{
    _gmouseWakeup(gm);
}

static gBool init_board(GMouse *m, unsigned driverinstance)
{
    gm = m;

    (void)	m;
    (void)	driverinstance;

    return gTrue;
}

static gBool read_xyz(GMouse *m, GMouseReading *prd)
{
    gU16 x;
    gU16 y;
    gU8 status;
    get_touch_x_y_status(&x, &y, &status);

    /*printf("x=%d, y=%d,status=%d \r\n", x, y, status);*/

    // Get the z reading (assumes we are ready to read z)
    // Take a shortcut and don't read x, y if we know we are definitely not touched.
    prd->x = x;
    prd->y = y;

    if (status) {
        prd->z = 1;
        prd->buttons = GINPUT_MOUSE_BTN_LEFT;

    } else {
        prd->z = 0;
        prd->buttons = 0;
    }
    return gTrue;
}

#endif /* _LLD_GMOUSE_MCU_BOARD_H */
