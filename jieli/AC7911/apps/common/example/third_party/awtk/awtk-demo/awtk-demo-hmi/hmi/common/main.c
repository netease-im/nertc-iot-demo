#include "awtk.h"
#include "mvvm_app.inc"

BEGIN_C_DECLS
#ifdef AWTK_WEB
#include "assets.inc"
#else /*AWTK_WEB*/
#include "../res/assets.inc"
#endif /*AWTK_WEB*/
END_C_DECLS

extern ret_t application_init(void);

extern ret_t application_exit(void);

#define APP_LCD_ORIENTATION LCD_ORIENTATION_0
#define APP_TYPE APP_SIMULATOR
#define APP_NAME "myapp"

#ifdef LINUX
#include <signal.h>
static void on_signal(int s)
{
    tk_quit();
}
#endif/*LINUX*/

static ret_t global_init(void)
{
    mvvm_app_init();
    mvvm_app_run_default_scripts();

#ifdef LINUX
    signal(SIGINT, on_signal);
#endif /*LINUX*/
    tk_socket_init();
    return RET_OK;
}


static ret_t global_deinit(void)
{
    mvvm_app_deinit();
    tk_socket_deinit();
    return RET_OK;
}

#define GLOBAL_INIT() global_init()
#define GLOBAL_EXIT() global_deinit()

#include "awtk_main.inc"
