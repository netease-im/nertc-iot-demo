/*********************************************************************************************
    *   Filename        : lib_system_config.c

    *   Description     : Optimized Code & RAM (编译优化配置)

    *   Author          : Bingquan

    *   Email           : caibingquan@zh-jieli.com

    *   Last modifiled  : 2019-03-18 15:22

    *   Copyright:(c)JIELI  2011-2019  @ , All Rights Reserved.
*********************************************************************************************/

#include "app_config.h"
#include "system/includes.h"

#ifdef CONFIG_SDFILE_EXT_ENABLE
const int config_sdfile_ext_enable = 1;
#else
const int config_sdfile_ext_enable = 0;
#endif

///打印是否时间打印信息
const int config_printf_time   = 1;

///异常中断，asser打印开启
#ifdef CONFIG_RELEASE_ENABLE
const int config_asser         = 1;
#else
const int config_asser         = 1;
#endif

const int config_system_info   = 0;

//为减少频繁写入VM导致的长时间关中断问题，提供VM写入缓存机制，调用vm_in_ram_update()才主动刷新到flash
const int config_vm_save_in_ram_enable = 0;

//================================================//
//                  SDFILE 精简使能               //
//================================================//
const int SDFILE_VFS_REDUCE_ENABLE = 0;

//================================================//
//                  FLASH FAT管理使能             //
//================================================//
#ifdef TCFG_VIRFAT_FLASH_SIMPLE
const int VIRFAT_FLASH_ENABLE = 1;
#else
const int VIRFAT_FLASH_ENABLE = 0;
#endif
const int FILT_0SIZE_ENABLE = 1; //是否过滤0大小文件
const int FILE_TIME_HIDDEN_ENABLE = 0;  //是否隐藏文件时间
const int FILE_TIME_USER_DEFINE_ENABLE = 0; //用户自行通过fat_set_datetime_info设置文件系统的时间
const int FATFS_TIMESORT_TURN_ENABLE = 0; //时排序翻转，由默认从小到大变成从大到小
const int FATFS_TIMESORT_NUM = 128; //按时间排序,记录文件数量
const int FATFS_SUPPORT_OVERSECTOR_RW = 1; //是否支持超过一个sector向设备拿数
const int FATFS_RW_MAX_CACHE = 32 * 1024; //设置读写申请的最大cache大小，小于512会被默认不生效
const int FILE_AUTO_RENAME_NUM = 1;  //自动重命名文件数量限制最大FILE_AUTO_RENAME_NUM * 8192个如果文件数量超最大以最大命名
const int FATFS_SUPPORT_WRITE_SAVE_MEANTIME = 0; //每次写同步目录项使能，会降低连续写速度。未fclose文件掉电场景使用
const int FATFS_SUPPORT_WRITE_CUTOFF = 1; //支持fseek截断文件，打开后fseek后指针位置决定文件大小
const int FATFS_GET_SPACE_USE_RAM = 0; //获取剩余容量使用大Buf缓存, 必须512倍数
const int FATFS_FORMAT_USE_RAM = 0; //32 * 1024;  //格式化功能使用大Buf缓存,加快速度, 必须512倍数
const int FILE_ALLOC_MODE = 1; //0表示和电脑逻辑一致，不节省空间和扫描时间。

//================================================//
//                  dev使用异步读使能             //
//================================================//
#ifdef TCFG_DEVICE_BULK_READ_ASYNC_ENABLE
const int device_bulk_read_async_enable = 1;
#else
const int device_bulk_read_async_enable = 0;
#endif

/**
 * @brief Log (Verbose/Info/Debug/Warn/Error)
 */
/*-----------------------------------------------------------*/
const char log_tag_const_v_SYS_TMR AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_SYS_TMR AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_SYS_TMR AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_SYS_TMR AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_SYS_TMR AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_WAIT_COMPLETION AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_WAIT_COMPLETION AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_WAIT_COMPLETION AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_WAIT_COMPLETION AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_WAIT_COMPLETION AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_SERVER_CORE AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_SERVER_CORE AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_SERVER_CORE AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_SERVER_CORE AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_SERVER_CORE AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_JLFS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_JLFS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_JLFS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_JLFS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_JLFS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_OS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_OS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_OS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_OS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_OS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_PORT AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_PORT AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_PORT AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_PORT AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_PORT AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_uECC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_uECC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_uECC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_uECC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_uECC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_HEAP_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_HEAP_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_HEAP_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_HEAP_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_HEAP_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_V_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_V_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_V_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_V_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_V_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_P_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_P_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_P_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_P_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_P_MEM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_APP_CORE AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_APP_CORE AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_APP_CORE AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_APP_CORE AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_APP_CORE AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_LBUF AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_LBUF AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_LBUF AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_LBUF AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_LBUF AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
