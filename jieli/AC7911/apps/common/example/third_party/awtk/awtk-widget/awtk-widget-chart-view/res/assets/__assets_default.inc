#include "awtk.h"
#include "base/assets_manager.h"
#if !defined(WITH_FS_RES) || defined(AWTK_WEB)
#include "default/inc/styles/default.data"
#include "default/inc/ui/home_page.data"
#include "default/inc/ui/system_bar.data"
#include "default/inc/ui/window_bar_series.data"
#include "default/inc/ui/window_bar_series_horizontal.data"
#include "default/inc/ui/window_bar_series_inverse.data"
#include "default/inc/ui/window_bar_series_minmax.data"
#include "default/inc/ui/window_bar_series_parall.data"
#include "default/inc/ui/window_line_series.data"
#include "default/inc/ui/window_line_series_category.data"
#include "default/inc/ui/window_line_series_colorful.data"
#include "default/inc/ui/window_line_series_cover.data"
#include "default/inc/ui/window_line_series_inverse.data"
#include "default/inc/ui/window_line_series_more_axis.data"
#include "default/inc/ui/window_line_series_parall.data"
#include "default/inc/ui/window_line_series_vertical.data"
#include "default/inc/ui/window_pie.data"
#include "default/inc/ui/window_tooltip_format.data"
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
  assets_manager_add(am, style_default_default);
  assets_manager_add(am, ui_home_page_default);
  assets_manager_add(am, ui_system_bar_default);
  assets_manager_add(am, ui_window_bar_series_default);
  assets_manager_add(am, ui_window_bar_series_horizontal_default);
  assets_manager_add(am, ui_window_bar_series_inverse_default);
  assets_manager_add(am, ui_window_bar_series_minmax_default);
  assets_manager_add(am, ui_window_bar_series_parall_default);
  assets_manager_add(am, ui_window_line_series_default);
  assets_manager_add(am, ui_window_line_series_category_default);
  assets_manager_add(am, ui_window_line_series_colorful_default);
  assets_manager_add(am, ui_window_line_series_cover_default);
  assets_manager_add(am, ui_window_line_series_inverse_default);
  assets_manager_add(am, ui_window_line_series_more_axis_default);
  assets_manager_add(am, ui_window_line_series_parall_default);
  assets_manager_add(am, ui_window_line_series_vertical_default);
  assets_manager_add(am, ui_window_pie_default);
  assets_manager_add(am, ui_window_tooltip_format_default);
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
