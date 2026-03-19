#include "awtk.h"

BEGIN_C_DECLS
#ifdef AWTK_WEB
#include "assets.inc"
#else /*AWTK_WEB*/
#if LCD_W == 480 || LCD_WIDTH == 480
#include "../res/assets_res_480_272.inc"
#else
#include "../res/assets.inc"
#endif
#endif /*AWTK_WEB*/
END_C_DECLS

extern ret_t application_init(void);

extern ret_t application_exit(void);

#include "awtk_main.inc"
