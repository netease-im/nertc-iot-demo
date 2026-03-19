#include "awtk.h"

BEGIN_C_DECLS
#ifdef AWTK_WEB
#include "assets.inc"
#else /*AWTK_WEB*/
/* #include "../res/assets.inc" */
#include "../res/assets_default.inc"
#endif /*AWTK_WEB*/
END_C_DECLS

extern ret_t application_init(void);

extern ret_t application_exit(void);

#include "awtk_main.inc"
