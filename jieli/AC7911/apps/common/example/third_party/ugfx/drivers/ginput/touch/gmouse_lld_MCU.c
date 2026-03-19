/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.io/license.html
 */

#include "gfx.h"

#if GFX_USE_GINPUT && GINPUT_NEED_MOUSE

#define GMOUSE_DRIVER_VMT		GMOUSEVMT_MCU
#include "../../../src/ginput/ginput_driver_mouse.h"

#include "gmouse_lld_MCU_board.h"

const GMouseVMT const GMOUSE_DRIVER_VMT[1] = {{
        {
            GDRIVER_TYPE_TOUCH,
            GMOUSE_VFLG_TOUCH | GMOUSE_VFLG_ONLY_DOWN,
            /*GMOUSE_VFLG_TOUCH | GMOUSE_VFLG_ONLY_DOWN | GMOUSE_VFLG_CALIBRATE | GMOUSE_VFLG_CAL_TEST,*/
            sizeof(GMouse) + GMOUSE_MCU_BOARD_DATA_SIZE,
            _gmouseInitDriver, _gmousePostInitDriver, _gmouseDeInitDriver
        },
        GMOUSE_MCU_Z_MAX,			// z_max
        GMOUSE_MCU_Z_MIN,			// z_min
        GMOUSE_MCU_Z_TOUCHON,		// z_touchon
        GMOUSE_MCU_Z_TOUCHOFF,		// z_touchoff
        {
            // pen_jitter
            GMOUSE_MCU_PEN_CALIBRATE_ERROR,			// calibrate
            GMOUSE_MCU_PEN_CLICK_ERROR,				// click
            GMOUSE_MCU_PEN_MOVE_ERROR				// move
        },
        {
            // finger_jitter
            GMOUSE_MCU_FINGER_CALIBRATE_ERROR,		// calibrate
            GMOUSE_MCU_FINGER_CLICK_ERROR,			// click
            GMOUSE_MCU_FINGER_MOVE_ERROR			// move
        },
        init_board,		// init
        0,				// deinit
        read_xyz,		// get
        0,				// calsave
        0				// calload
    }
};
#else
void ugfx_touch_event_process(u16 x, u16 y, u8 event_press) {}
#endif /* GFX_USE_GINPUT && GINPUT_NEED_MOUSE */
