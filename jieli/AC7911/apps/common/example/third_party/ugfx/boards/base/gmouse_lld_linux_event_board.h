/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.io/license.html
 */

#ifndef _GINPUT_LLD_MOUSE_BOARD_H
#define _GINPUT_LLD_MOUSE_BOARD_H

// Set this to GFXON if you want self-calibration.
//	NOTE:	This is not as accurate as real calibration.
//			It requires the orientation of the touch panel to match the display.
//			It requires the active area of the touch panel to exactly match the display size.
#define GMOUSE_LINUX_EVENT_SELF_CALIBRATE			GFXOFF

#define GMOUSE_LINUX_EVENT_FINGERMODE				GFXON

#endif /* _GINPUT_LLD_MOUSE_BOARD_H */
