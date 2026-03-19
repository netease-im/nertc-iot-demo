#include "awtk.h"
#include "base/assets_manager.h"
#if !defined(WITH_FS_RES) || defined(AWTK_WEB)
#include "default/inc/strings/en_US.data"
#include "default/inc/strings/zh_CN.data"
#include "default/inc/styles/default.data"
#include "default/inc/styles/keyboard.data"
#include "default/inc/styles/main.data"
#include "default/inc/ui/kb_ascii.data"
#include "default/inc/ui/kb_default.data"
#include "default/inc/ui/kb_float.data"
#include "default/inc/ui/kb_hex.data"
#include "default/inc/ui/kb_int.data"
#include "default/inc/ui/kb_phone.data"
#include "default/inc/ui/kb_ufloat.data"
#include "default/inc/ui/kb_uint.data"
#include "default/inc/ui/scores_add.data"
#include "default/inc/ui/scores_detail.data"
#include "default/inc/ui/scores_edit.data"
#include "default/inc/ui/student_list.data"
#include "default/inc/ui/student_list_mvvm.data"
#include "default/inc/data/students_db.data"
#ifndef AWTK_WEB
#ifdef WITH_STB_IMAGE
#else /*WITH_STB_IMAGE*/
#endif /*WITH_STB_IMAGE*/
#ifdef WITH_TRUETYPE_FONT
#else /*WITH_TRUETYPE_FONT*/
#endif /*WITH_TRUETYPE_FONT*/
#endif /*AWTK_WEB*/
#ifdef WITH_VGCANVAS
#endif /*WITH_VGCANVAS*/
#endif /*!defined(WITH_FS_RES) || defined(AWTK_WEB)*/

ret_t assets_init_default(void) {
  assets_manager_t* am = assets_manager();
  assets_manager_set_theme(am, "default");

#if defined(WITH_FS_RES) && !defined(AWTK_WEB)
  assets_manager_preload(am, ASSET_TYPE_STYLE, "default");
#else /*defined(WITH_FS_RES) && !defined(AWTK_WEB)*/
  assets_manager_add(am, strings_en_US_default);
  assets_manager_add(am, strings_zh_CN_default);
  assets_manager_add(am, style_default_default);
  assets_manager_add(am, style_keyboard_default);
  assets_manager_add(am, style_main_default);
  assets_manager_add(am, ui_kb_ascii_default);
  assets_manager_add(am, ui_kb_default_default);
  assets_manager_add(am, ui_kb_float_default);
  assets_manager_add(am, ui_kb_hex_default);
  assets_manager_add(am, ui_kb_int_default);
  assets_manager_add(am, ui_kb_phone_default);
  assets_manager_add(am, ui_kb_ufloat_default);
  assets_manager_add(am, ui_kb_uint_default);
  assets_manager_add(am, ui_scores_add_default);
  assets_manager_add(am, ui_scores_detail_default);
  assets_manager_add(am, ui_scores_edit_default);
  assets_manager_add(am, ui_student_list_default);
  assets_manager_add(am, ui_student_list_mvvm_default);
  assets_manager_add(am, data_students_db_default);
#ifndef AWTK_WEB
#ifdef WITH_TRUETYPE_FONT
#else /*WITH_TRUETYPE_FONT*/
#endif /*WITH_TRUETYPE_FONT*/
#endif /*AWTK_WEB*/
#ifdef WITH_VGCANVAS
#endif /*WITH_VGCANVAS*/
#endif /*defined(WITH_FS_RES) && !defined(AWTK_WEB)*/

  tk_init_assets();
  return RET_OK;
}
