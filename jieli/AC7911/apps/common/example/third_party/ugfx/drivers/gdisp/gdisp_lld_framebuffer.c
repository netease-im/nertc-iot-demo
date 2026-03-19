/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.io/license.html
 */

#include "gfx.h"

#if GFX_USE_GDISP

#define GDISP_DRIVER_VMT			GDISPVMT_framebuffer
#include "gdisp_lld_config.h"
#include "../../../src/gdisp/gdisp_driver.h"

typedef struct fbInfo {
    gU8            idx;
    void 			*pixels[2];			// The pixel buffer
    gCoord x1[2];
    gCoord y1[2];
    gCoord x2[2];
    gCoord y2[2];
    gCoord			linelen;		// The number of bytes per display line
} fbInfo;

typedef struct fbPriv {
    fbInfo			fbi;			// Display information
} fbPriv;
#define PRIV(g) ((fbPriv *)g->priv)
#define GDISP_FLG_NEEDFLUSH		(GDISP_FLG_DRIVER << 0)

#include "board_framebuffer.h"

/*===========================================================================*/
/* Driver local routines    .                                                */
/*===========================================================================*/

#define PIXIL_POS(g, x, y)		((y) * ((fbPriv *)(g)->priv)->fbi.linelen + (x) * sizeof(LLDCOLOR_TYPE))
#define PIXEL_ADDR_WRITE(g, pos)	((LLDCOLOR_TYPE *)(((char *)((fbPriv *)(g)->priv)->fbi.pixels[((fbPriv *)(g)->priv)->fbi.idx])+pos))
#define PIXEL_ADDR_READ(g, pos)		((LLDCOLOR_TYPE *)(((char *)((fbPriv *)(g)->priv)->fbi.pixels[!((fbPriv *)(g)->priv)->fbi.idx])+pos))

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

LLDSPEC gBool gdisp_lld_init(GDisplay *g)
{

    // Initialize the private structure
    if (!(g->priv = gfxAlloc(sizeof(fbPriv)))) {
        gfxHalt("GDISP Framebuffer: Failed to allocate private memory");
    }

    // Initialize the GDISP structure
    g->g.Orientation = gOrientation0;
    g->g.Powermode = gPowerOn;
    g->board = 0;							// preinitialize
    board_init(g, &PRIV(g)->fbi);

    return gTrue;
}

#if GDISP_HARDWARE_FLUSH
LLDSPEC void gdisp_lld_flush(GDisplay *g)
{
    fbInfo *fbi = &(PRIV(g)->fbi);

    // Don't flush unless we really need to
    if (!(g->flags & GDISP_FLG_NEEDFLUSH)) {
        return;
    }

    board_flush(g);

    // Clear the 'need-flushing' flag and reset the window
    g->flags &= ~GDISP_FLG_NEEDFLUSH;
    fbi->x1[fbi->idx] = g->g.Width;
    fbi->y1[fbi->idx] = g->g.Height;
    fbi->x2[fbi->idx] = -1;
    fbi->y2[fbi->idx] = -1;
}
#endif

LLDSPEC void gdisp_lld_draw_pixel(GDisplay *g)
{
    fbInfo *fbi = &(PRIV(g)->fbi);

    unsigned	pos;

#if GDISP_NEED_CONTROL
    switch (g->g.Orientation) {
    case gOrientation0:
    default:
        pos = PIXIL_POS(g, g->p.x, g->p.y);
        break;
    case gOrientation90:
        pos = PIXIL_POS(g, g->p.y, g->g.Width - g->p.x - 1);
        break;
    case gOrientation180:
        pos = PIXIL_POS(g, g->g.Width - g->p.x - 1, g->g.Height - g->p.y - 1);
        break;
    case gOrientation270:
        pos = PIXIL_POS(g, g->g.Height - g->p.y - 1, g->p.x);
        break;
    }
#else
    pos = PIXIL_POS(g, g->p.x, g->p.y);
#endif

    PIXEL_ADDR_WRITE(g, pos)[0] = gdispColor2Native(g->p.color);

    // update flush window boundaries
    if (g->p.x < fbi->x1[fbi->idx]) {
        fbi->x1[fbi->idx] = g->p.x;
    }
    if (g->p.x > fbi->x2[fbi->idx]) {
        fbi->x2[fbi->idx] = g->p.x;
    }
    if (g->p.y < fbi->y1[fbi->idx]) {
        fbi->y1[fbi->idx] = g->p.y;
    }
    if (g->p.y > fbi->y2[fbi->idx]) {
        fbi->y2[fbi->idx] = g->p.y;
    }
    g->flags |= GDISP_FLG_NEEDFLUSH;
}

#if GDISP_HARDWARE_PIXELREAD
LLDSPEC	gColor gdisp_lld_get_pixel_color(GDisplay *g)
{
    unsigned		pos;
    LLDCOLOR_TYPE	color;

#if GDISP_NEED_CONTROL
    switch (g->g.Orientation) {
    case gOrientation0:
    default:
        pos = PIXIL_POS(g, g->p.x, g->p.y);
        break;
    case gOrientation90:
        pos = PIXIL_POS(g, g->p.y, g->g.Width - g->p.x - 1);
        break;
    case gOrientation180:
        pos = PIXIL_POS(g, g->g.Width - g->p.x - 1, g->g.Height - g->p.y - 1);
        break;
    case gOrientation270:
        pos = PIXIL_POS(g, g->g.Height - g->p.y - 1, g->p.x);
        break;
    }
#else
    pos = PIXIL_POS(g, g->p.x, g->p.y);
#endif

    color = PIXEL_ADDR_READ(g, pos)[0];
    return gdispNative2Color(color);
}
#endif

#if GDISP_NEED_CONTROL
LLDSPEC void gdisp_lld_control(GDisplay *g)
{
    switch (g->p.x) {
    case GDISP_CONTROL_POWER:
        if (g->g.Powermode == (gPowermode)g->p.ptr) {
            return;
        }
        switch ((gPowermode)g->p.ptr) {
        case gPowerOff:
        case gPowerOn:
        case gPowerSleep:
        case gPowerDeepSleep:
            board_power(g, (gPowermode)g->p.ptr);
            break;
        default:
            return;
        }
        g->g.Powermode = (gPowermode)g->p.ptr;
        return;

    case GDISP_CONTROL_ORIENTATION:
        if (g->g.Orientation == (gOrientation)g->p.ptr) {
            return;
        }
        switch ((gOrientation)g->p.ptr) {
        case gOrientation0:
        case gOrientation180:
            if (g->g.Orientation == gOrientation90 || g->g.Orientation == gOrientation270) {
                gCoord		tmp;

                tmp = g->g.Width;
                g->g.Width = g->g.Height;
                g->g.Height = tmp;
            }
            break;
        case gOrientation90:
        case gOrientation270:
            if (g->g.Orientation == gOrientation0 || g->g.Orientation == gOrientation180) {
                gCoord		tmp;

                tmp = g->g.Width;
                g->g.Width = g->g.Height;
                g->g.Height = tmp;
            }
            break;
        default:
            return;
        }
        g->g.Orientation = (gOrientation)g->p.ptr;
        return;

    case GDISP_CONTROL_BACKLIGHT:
        if ((unsigned)g->p.ptr > 100) {
            g->p.ptr = (void *)100;
        }
        board_backlight(g, (unsigned)g->p.ptr);
        g->g.Backlight = (unsigned)g->p.ptr;
        return;

    case GDISP_CONTROL_CONTRAST:
        if ((unsigned)g->p.ptr > 100) {
            g->p.ptr = (void *)100;
        }
        board_contrast(g, (unsigned)g->p.ptr);
        g->g.Contrast = (unsigned)g->p.ptr;
        return;
    }
}
#endif

#endif /* GFX_USE_GDISP */
