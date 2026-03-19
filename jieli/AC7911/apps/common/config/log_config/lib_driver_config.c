/*********************************************************************************************
    *   Filename        : lib_driver_config.c

    *   Description     : Optimized Code & RAM (编译优化配置)

    *   Author          : Bingquan

    *   Email           : caibingquan@zh-jieli.com

    *   Last modifiled  : 2019-03-18 14:58

    *   Copyright:(c)JIELI  2011-2019  @ , All Rights Reserved.
*********************************************************************************************/
#include "app_config.h"
#include "system/includes.h"

#ifdef CONFIG_SFC_ENABLE
const int norflash_write_cpu_unhold = 0;//在 跑SFC的情况下 写flash hold cpu,关闭所有中断
#else
const int norflash_write_cpu_unhold = 1;//在跑SDRAM的情况下 写flash 不需要 hold cpu和关闭所有中断 ,程序员自己懂得把相关text段放到sdram/内部ram,保证运行过程中不会访问SFC也可以写1
#endif

const int norflash_write_readback_check_en = 0;// norflash写完后回读校验使能开关 0-关, 1-开

const int config_spi_code_user_cache = 0;

const int sdx_idle_query_check_keep_poweron = 0;	//如果硬件设计上，当进入低功耗时会导致sd卡电源会掉时，需要使能此变量（如果采用命令检测方式不要使能此变量）

#if TCFG_SD0_SD1_USE_THE_SAME_HW
const int sd0_sd1_use_the_same_hw = 1;
#else
const int sd0_sd1_use_the_same_hw = 0;
#endif

#if TCFG_KEEP_CARD_AT_ACTIVE_STATUS
const int keep_card_at_active_status = 1;
#else
const int keep_card_at_active_status = 0;
#endif

#if TCFG_SDX_CAN_OPERATE_MMC_CARD
const int sdx_can_operate_mmc_card = 1;
#else
const int sdx_can_operate_mmc_card = 0;
#endif

#if (TCFG_CLOCK_SYS_SRC == SYS_CLOCK_INPUT_PLL_RCL) //系统时钟源选择
const int clock_sys_src_use_lrc_hw = 1; //当使用lrc时timer.c需要特殊设置
#else
const int clock_sys_src_use_lrc_hw = 0;
#endif

//config_otg_slave_detect_method_2为1时，从机检测的方式使用方法2
const int config_otg_slave_detect_method_2 = 0;
const u32 otg_slave_detect_dm_delay = 50;
const u32 otg_slave_detect_dm_delay_2 = 1;

/**
 * @brief Log (Verbose/Info/Debug/Warn/Error)
 */
/*-----------------------------------------------------------*/
const char log_tag_const_v_CLOCK AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_CLOCK AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_CLOCK AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_CLOCK AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_CLOCK AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_LP_TIMER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_LP_TIMER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_LP_TIMER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_LP_TIMER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_LP_TIMER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_LRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_LRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_LRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_LRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_LRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_P33 AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_P33 AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_d_P33 AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_P33 AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_P33 AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_PMU AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_PMU AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_PMU AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_PMU AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_PMU AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_WKUP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_WKUP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_WKUP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_WKUP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_WKUP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_SDFILE AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_SDFILE AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_SDFILE AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_SDFILE AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_SDFILE AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_DEBUG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_DEBUG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_DEBUG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_DEBUG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_DEBUG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_PWM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_PWM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_PWM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_PWM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_PWM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_KEY AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_KEY AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_KEY AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_KEY AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_KEY AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_TMR AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_TMR AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_TMR AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_TMR AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_TMR AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_VM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_VM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_VM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_VM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_VM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_USB AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_d_USB AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_i_USB AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_USB AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_USB AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_SDRAM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_SDRAM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_SDRAM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_SDRAM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_SDRAM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_SD AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_SD AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_SD AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_SD AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_SD AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_SPI AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_SPI AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_SPI AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_SPI AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_SPI AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_RTC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_RTC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_RTC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_RTC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_RTC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_DMA AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_DMA AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_DMA AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_DMA AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_DMA AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_UART AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_UART AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_UART AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_UART AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_UART AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_WLC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_WLC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_WLC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_WLC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_WLC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_AUDIO_ADC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_AUDIO_ADC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_AUDIO_ADC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_AUDIO_ADC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_AUDIO_ADC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_AUDIO_DAC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_AUDIO_DAC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_AUDIO_DAC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_AUDIO_DAC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_AUDIO_DAC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_AUDIO_IIS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_AUDIO_IIS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_AUDIO_IIS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_AUDIO_IIS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_AUDIO_IIS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_AUDIO_PLNK AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_AUDIO_PLNK AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_AUDIO_PLNK AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_AUDIO_PLNK AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_AUDIO_PLNK AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_AUDIO_SBC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_AUDIO_SBC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_AUDIO_SBC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_AUDIO_SBC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_AUDIO_SBC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_AUDIO_SPDIF AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_AUDIO_SPDIF AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_AUDIO_SPDIF AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_AUDIO_SPDIF AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_AUDIO_SPDIF AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_AUDIO_DEV AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_AUDIO_DEV AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_AUDIO_DEV AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_AUDIO_DEV AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_AUDIO_DEV AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_EMI AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_EMI AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_EMI AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_EMI AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_EMI AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_IMD AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_IMD AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_IMD AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_IMD AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_IMD AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_PAP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_PAP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_PAP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_PAP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_PAP AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_VIDEO AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_VIDEO AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_VIDEO AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_VIDEO AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_VIDEO AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_VIDEO_UVC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_VIDEO_UVC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_VIDEO_UVC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_VIDEO_UVC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_VIDEO_UVC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_VIDEO_JPEG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_VIDEO_JPEG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_VIDEO_JPEG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_VIDEO_JPEG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_VIDEO_JPEG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_VIDEO_ISC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_VIDEO_ISC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_VIDEO_ISC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_VIDEO_ISC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_VIDEO_ISC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_VIDEO_IMG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_VIDEO_IMG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_VIDEO_IMG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_VIDEO_IMG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_VIDEO_IMG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_VIDEO_CAMERA AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_VIDEO_CAMERA AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_VIDEO_CAMERA AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_VIDEO_CAMERA AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_VIDEO_CAMERA AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_VIDEO_DEC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_VIDEO_DEC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_VIDEO_DEC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_VIDEO_DEC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_VIDEO_DEC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_VIDEO_REC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_VIDEO_REC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_VIDEO_REC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_w_VIDEO_REC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_e_VIDEO_REC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
