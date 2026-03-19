#include "awtk.h"
#include "base/assets_manager.h"
#if !defined(WITH_FS_RES) || defined(AWTK_WEB)
#include "assets/default/inc/styles/default.data"
#include "assets/default/inc/styles/main.data"
#include "assets/default/inc/ui/main.data"
#ifndef AWTK_WEB
#ifdef WITH_STB_IMAGE
#include "assets/default/inc/images/message.res"
#else /*WITH_STB_IMAGE*/
#endif /*WITH_STB_IMAGE*/
#ifdef WITH_TRUETYPE_FONT
#include "assets/default/inc/fonts/default.res"
#else /*WITH_TRUETYPE_FONT*/
#endif /*WITH_TRUETYPE_FONT*/
#endif /*AWTK_WEB*/
#ifdef WITH_VGCANVAS
#endif /*WITH_VGCANVAS*/
#endif /*!defined(WITH_FS_RES) || defined(AWTK_WEB)*/

ret_t assets_init(void) {
  assets_manager_t* am = assets_manager();
  assets_manager_set_theme(am, "default");

#if defined(WITH_FS_RES) && !defined(AWTK_WEB)
  assets_manager_preload(am, ASSET_TYPE_STYLE, "default");
#else /*defined(WITH_FS_RES) && !defined(AWTK_WEB)*/
  assets_manager_add(am, style_default);
  assets_manager_add(am, style_main);
  assets_manager_add(am, ui_main);
#ifndef AWTK_WEB
  assets_manager_add(am, image_message);
#ifdef WITH_TRUETYPE_FONT
  assets_manager_add(am, font_default);
#else /*WITH_TRUETYPE_FONT*/
#endif /*WITH_TRUETYPE_FONT*/
#endif /*AWTK_WEB*/
#ifdef WITH_VGCANVAS
#endif /*WITH_VGCANVAS*/
#endif /*defined(WITH_FS_RES) && !defined(AWTK_WEB)*/

  tk_init_assets();
  return RET_OK;
}
