#include "awtk.h"
#include "base/assets_manager.h"
#if !defined(WITH_FS_RES) || defined(AWTK_WEB)
extern TK_CONST_DATA_ALIGN(const unsigned char strings_en_US_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char strings_zh_CN_default[]);
#include "dark/inc/styles/system_bar.data"
#include "dark/inc/styles/temperature40.data"
extern TK_CONST_DATA_ALIGN(const unsigned char style_default_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char style_dialog_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char style_dialog_confirm_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char style_dialog_info_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char style_dialog_toast_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char style_dialog_warn_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char style_file_browser_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char style_keyboard_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char style_main_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char style_window1_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_address_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_app_conf_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_app_conf_array_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_app_conf_extend_cmds_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_app_conf_main_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_app_conf_reload_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_app_conf_sub_vm_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_books13_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_books39_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_calculator_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_compositor_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_com_settings_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_count_down_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_custom_handlers_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_demo33_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_demo33_a_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_demo33_b_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_demo35_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_demo9_main_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_file_chooser_for_open_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_file_chooser_for_save_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_folder_chooser_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_home_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_humidity_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_image_animation_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_iotjs_fs_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_js_temperature40_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_kb_ascii_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_kb_default_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_kb_float_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_kb_hex_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_kb_int_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_kb_phone_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_kb_ufloat_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_kb_uint_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_network_add_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_network_edit_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_room_settings_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_screensaver_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_send_key_set_prop_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_shape_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_system_bar_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_temperature1_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_temperature10_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_temperature14_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_temperature16_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_temperature17_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_temperature18_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_temperature19_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_temperature2_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_temperature28_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_temperature28_home_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_temperature3_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_temperature4_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_temperature40_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_temperature5_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_temperature6_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_temperature9_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_temperature_humidity_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_temperature_humidity2_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_temperature_humidity3_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char data_a_b_c_any_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char data_com_zlg_app_json_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char data_demo23_json_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char data_demo25_json_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char data_demo29_json_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char data_file_info_txt_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char data_gpinyin_dat_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char data_suggest_words_zh_cn_dat_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char data_test_dat_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char data_test_json_default[]);
#ifndef AWTK_WEB
#ifdef WITH_STB_IMAGE
#else /*WITH_STB_IMAGE*/
#endif /*WITH_STB_IMAGE*/
#ifdef WITH_TRUETYPE_FONT
#else /*WITH_TRUETYPE_FONT*/
#endif /*WITH_TRUETYPE_FONT*/
#endif /*AWTK_WEB*/
#ifdef WITH_VGCANVAS
extern TK_CONST_DATA_ALIGN(const unsigned char image_ball_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_china_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_girl_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_pointer_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_pointer_1_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_pointer_4_default[]);
#endif /*WITH_VGCANVAS*/
#endif /*!defined(WITH_FS_RES) || defined(AWTK_WEB)*/

ret_t assets_init_dark(void) {
  assets_manager_t* am = assets_manager();
  assets_manager_set_theme(am, "dark");

#if defined(WITH_FS_RES) && !defined(AWTK_WEB)
  assets_manager_preload(am, ASSET_TYPE_STYLE, "default");
#else /*defined(WITH_FS_RES) && !defined(AWTK_WEB)*/
  assets_manager_add(am, strings_en_US_default);
  assets_manager_add(am, strings_zh_CN_default);
  assets_manager_add(am, style_system_bar_dark);
  assets_manager_add(am, style_temperature40_dark);
  assets_manager_add(am, style_default_default);
  assets_manager_add(am, style_dialog_default);
  assets_manager_add(am, style_dialog_confirm_default);
  assets_manager_add(am, style_dialog_info_default);
  assets_manager_add(am, style_dialog_toast_default);
  assets_manager_add(am, style_dialog_warn_default);
  assets_manager_add(am, style_file_browser_default);
  assets_manager_add(am, style_keyboard_default);
  assets_manager_add(am, style_main_default);
  assets_manager_add(am, style_window1_default);
  assets_manager_add(am, ui_address_default);
  assets_manager_add(am, ui_app_conf_default);
  assets_manager_add(am, ui_app_conf_array_default);
  assets_manager_add(am, ui_app_conf_extend_cmds_default);
  assets_manager_add(am, ui_app_conf_main_default);
  assets_manager_add(am, ui_app_conf_reload_default);
  assets_manager_add(am, ui_app_conf_sub_vm_default);
  assets_manager_add(am, ui_books13_default);
  assets_manager_add(am, ui_books39_default);
  assets_manager_add(am, ui_calculator_default);
  assets_manager_add(am, ui_compositor_default);
  assets_manager_add(am, ui_com_settings_default);
  assets_manager_add(am, ui_count_down_default);
  assets_manager_add(am, ui_custom_handlers_default);
  assets_manager_add(am, ui_demo33_default);
  assets_manager_add(am, ui_demo33_a_default);
  assets_manager_add(am, ui_demo33_b_default);
  assets_manager_add(am, ui_demo35_default);
  assets_manager_add(am, ui_demo9_main_default);
  assets_manager_add(am, ui_file_chooser_for_open_default);
  assets_manager_add(am, ui_file_chooser_for_save_default);
  assets_manager_add(am, ui_folder_chooser_default);
  assets_manager_add(am, ui_home_default);
  assets_manager_add(am, ui_humidity_default);
  assets_manager_add(am, ui_image_animation_default);
  assets_manager_add(am, ui_iotjs_fs_default);
  assets_manager_add(am, ui_js_temperature40_default);
  assets_manager_add(am, ui_kb_ascii_default);
  assets_manager_add(am, ui_kb_default_default);
  assets_manager_add(am, ui_kb_float_default);
  assets_manager_add(am, ui_kb_hex_default);
  assets_manager_add(am, ui_kb_int_default);
  assets_manager_add(am, ui_kb_phone_default);
  assets_manager_add(am, ui_kb_ufloat_default);
  assets_manager_add(am, ui_kb_uint_default);
  assets_manager_add(am, ui_network_add_default);
  assets_manager_add(am, ui_network_edit_default);
  assets_manager_add(am, ui_room_settings_default);
  assets_manager_add(am, ui_screensaver_default);
  assets_manager_add(am, ui_send_key_set_prop_default);
  assets_manager_add(am, ui_shape_default);
  assets_manager_add(am, ui_system_bar_default);
  assets_manager_add(am, ui_temperature1_default);
  assets_manager_add(am, ui_temperature10_default);
  assets_manager_add(am, ui_temperature14_default);
  assets_manager_add(am, ui_temperature16_default);
  assets_manager_add(am, ui_temperature17_default);
  assets_manager_add(am, ui_temperature18_default);
  assets_manager_add(am, ui_temperature19_default);
  assets_manager_add(am, ui_temperature2_default);
  assets_manager_add(am, ui_temperature28_default);
  assets_manager_add(am, ui_temperature28_home_default);
  assets_manager_add(am, ui_temperature3_default);
  assets_manager_add(am, ui_temperature4_default);
  assets_manager_add(am, ui_temperature40_default);
  assets_manager_add(am, ui_temperature5_default);
  assets_manager_add(am, ui_temperature6_default);
  assets_manager_add(am, ui_temperature9_default);
  assets_manager_add(am, ui_temperature_humidity_default);
  assets_manager_add(am, ui_temperature_humidity2_default);
  assets_manager_add(am, ui_temperature_humidity3_default);
  assets_manager_add(am, data_a_b_c_any_default);
  assets_manager_add(am, data_com_zlg_app_json_default);
  assets_manager_add(am, data_demo23_json_default);
  assets_manager_add(am, data_demo25_json_default);
  assets_manager_add(am, data_demo29_json_default);
  assets_manager_add(am, data_file_info_txt_default);
  assets_manager_add(am, data_gpinyin_dat_default);
  assets_manager_add(am, data_suggest_words_zh_cn_dat_default);
  assets_manager_add(am, data_test_dat_default);
  assets_manager_add(am, data_test_json_default);
#ifndef AWTK_WEB
#ifdef WITH_TRUETYPE_FONT
#else /*WITH_TRUETYPE_FONT*/
#endif /*WITH_TRUETYPE_FONT*/
#endif /*AWTK_WEB*/
#ifdef WITH_VGCANVAS
  assets_manager_add(am, image_ball_default);
  assets_manager_add(am, image_china_default);
  assets_manager_add(am, image_girl_default);
  assets_manager_add(am, image_pointer_default);
  assets_manager_add(am, image_pointer_1_default);
  assets_manager_add(am, image_pointer_4_default);
#endif /*WITH_VGCANVAS*/
#endif /*defined(WITH_FS_RES) && !defined(AWTK_WEB)*/

  tk_init_assets();
  return RET_OK;
}
