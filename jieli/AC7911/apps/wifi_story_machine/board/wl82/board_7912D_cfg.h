#ifndef CONFIG_BOARD_7912D_CFG_H
#define CONFIG_BOARD_7912D_CFG_H

#ifdef CONFIG_BOARD_7912D

#define __FLASH_SIZE__    (2 * 1024 * 1024)
#define __SDRAM_SIZE__    (0 * 1024 * 1024)

#undef CONFIG_VIDEO_ENABLE
#undef CONFIG_AUDIO_ENABLE
#define TCFG_SD0_ENABLE                     1
#define TCFG_ADKEY_ENABLE                   1          //AD按键
#if TCFG_ADKEY_ENABLE
#define CONFIG_PRESS_LONG_KEY_POWERON                  //长按开关机功能
#endif

#define TCFG_DEBUG_PORT                     IO_PORTA_03

#define TCFG_SD_PORTS                       'B'			//SD0/SD1的ABCD组(默认为开发板SD0-D),注意:IO占用问题
#define TCFG_SD_DAT_WIDTH                   1			//1:单线模式, 4:四线模式
#define TCFG_SD_DET_MODE                    SD_CLK_DECT	//检测模式:命令检测，时钟检测，IO检测
#define TCFG_SD_DET_IO                      IO_PORTB_08	//SD_DET_MODE为SD_IO_DECT时有效
#define TCFG_SD_DET_IO_LEVEL                0			//IO检卡上线的电平(0/1),SD_DET_MODE为SD_IO_DECT时有效
#define TCFG_SD_CLK                         24000000	//SD时钟

//*********************************************************************************//
//                            AUDIO_ADC应用的通道配置                              //
//*********************************************************************************//
#define CONFIG_AUDIO_ENC_SAMPLE_SOURCE      AUDIO_ENC_SAMPLE_SOURCE_MIC

#ifdef CONFIG_BOARD_MUTEX
#error "board config can not enable at the same time, just select one!!!"
#else
#define CONFIG_BOARD_MUTEX
#endif

#endif

#endif
