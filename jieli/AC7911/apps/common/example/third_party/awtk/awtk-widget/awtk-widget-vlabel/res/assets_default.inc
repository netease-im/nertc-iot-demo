#include "awtk.h"
#include "base/assets_manager.h"
#if !defined(WITH_FS_RES) || defined(AWTK_WEB)
#include "assets/default/inc/strings/en_US.data"
#include "assets/default/inc/strings/zh_CN.data"
#include "assets/default/inc/styles/default.data"
#include "assets/default/inc/styles/keyboard.data"
#include "assets/default/inc/styles/main.data"
#include "assets/default/inc/ui/kb_ascii.data"
#include "assets/default/inc/ui/kb_default.data"
#include "assets/default/inc/ui/kb_float.data"
#include "assets/default/inc/ui/kb_hex.data"
#include "assets/default/inc/ui/kb_int.data"
#include "assets/default/inc/ui/kb_phone.data"
#include "assets/default/inc/ui/kb_ufloat.data"
#include "assets/default/inc/ui/kb_uint.data"
#include "assets/default/inc/ui/main.data"
#ifndef AWTK_WEB
#ifdef WITH_STB_IMAGE
#include "assets/default/inc/images/arrow_down_d.res"
#include "assets/default/inc/images/arrow_down_f.res"
#include "assets/default/inc/images/arrow_down_n.res"
#include "assets/default/inc/images/arrow_down_o.res"
#include "assets/default/inc/images/arrow_down_p.res"
#include "assets/default/inc/images/arrow_left_d.res"
#include "assets/default/inc/images/arrow_left_f.res"
#include "assets/default/inc/images/arrow_left_n.res"
#include "assets/default/inc/images/arrow_left_o.res"
#include "assets/default/inc/images/arrow_left_p.res"
#include "assets/default/inc/images/arrow_right_d.res"
#include "assets/default/inc/images/arrow_right_f.res"
#include "assets/default/inc/images/arrow_right_n.res"
#include "assets/default/inc/images/arrow_right_o.res"
#include "assets/default/inc/images/arrow_right_p.res"
#include "assets/default/inc/images/arrow_up_d.res"
#include "assets/default/inc/images/arrow_up_f.res"
#include "assets/default/inc/images/arrow_up_n.res"
#include "assets/default/inc/images/arrow_up_o.res"
#include "assets/default/inc/images/arrow_up_p.res"
#include "assets/default/inc/images/check_checked.res"
#include "assets/default/inc/images/check_checked_d.res"
#include "assets/default/inc/images/check_unchecked.res"
#include "assets/default/inc/images/check_unchecked_d.res"
#include "assets/default/inc/images/radio_checked.res"
#include "assets/default/inc/images/radio_checked_d.res"
#include "assets/default/inc/images/radio_checked_d_d.res"
#include "assets/default/inc/images/radio_unchecked.res"
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
  assets_manager_add(am, strings_en_US);
  assets_manager_add(am, strings_zh_CN);
  assets_manager_add(am, style_default);
  assets_manager_add(am, style_keyboard);
  assets_manager_add(am, style_main);
  assets_manager_add(am, ui_kb_ascii);
  assets_manager_add(am, ui_kb_default);
  assets_manager_add(am, ui_kb_float);
  assets_manager_add(am, ui_kb_hex);
  assets_manager_add(am, ui_kb_int);
  assets_manager_add(am, ui_kb_phone);
  assets_manager_add(am, ui_kb_ufloat);
  assets_manager_add(am, ui_kb_uint);
  assets_manager_add(am, ui_main);
#ifndef AWTK_WEB
  assets_manager_add(am, image_arrow_down_d);
  assets_manager_add(am, image_arrow_down_f);
  assets_manager_add(am, image_arrow_down_n);
  assets_manager_add(am, image_arrow_down_o);
  assets_manager_add(am, image_arrow_down_p);
  assets_manager_add(am, image_arrow_left_d);
  assets_manager_add(am, image_arrow_left_f);
  assets_manager_add(am, image_arrow_left_n);
  assets_manager_add(am, image_arrow_left_o);
  assets_manager_add(am, image_arrow_left_p);
  assets_manager_add(am, image_arrow_right_d);
  assets_manager_add(am, image_arrow_right_f);
  assets_manager_add(am, image_arrow_right_n);
  assets_manager_add(am, image_arrow_right_o);
  assets_manager_add(am, image_arrow_right_p);
  assets_manager_add(am, image_arrow_up_d);
  assets_manager_add(am, image_arrow_up_f);
  assets_manager_add(am, image_arrow_up_n);
  assets_manager_add(am, image_arrow_up_o);
  assets_manager_add(am, image_arrow_up_p);
  assets_manager_add(am, image_check_checked);
  assets_manager_add(am, image_check_checked_d);
  assets_manager_add(am, image_check_unchecked);
  assets_manager_add(am, image_check_unchecked_d);
  assets_manager_add(am, image_radio_checked);
  assets_manager_add(am, image_radio_checked_d);
  assets_manager_add(am, image_radio_checked_d_d);
  assets_manager_add(am, image_radio_unchecked);
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
