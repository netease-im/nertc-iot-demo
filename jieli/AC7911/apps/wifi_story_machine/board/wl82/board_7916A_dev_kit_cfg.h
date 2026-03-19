#ifndef CONFIG_BOARD_7916A_DEV_KIT_CFG_H
#define CONFIG_BOARD_7916A_DEV_KIT_CFG_H

#ifdef CONFIG_BOARD_DEV_KIT		//开源学习开发板

#define CONFIG_SDRAM_OVERCLOCKING_ENABLE

#define TCFG_SD0_ENABLE                     1
#define TCFG_ADKEY_ENABLE                   1           //AD按键
#if TCFG_ADKEY_ENABLE
#define CONFIG_PRESS_LONG_KEY_POWERON                   //长按开关机功能
#endif
#define CONFIG_FLASH_FOUR_WIDTH_ENABLE                  //FLASH四线模式
// #define CONFIG_OVERCLOCKING_ENABLE                      //超频模式，暂时不能量产，仅供测试
#define CONFIG_CAMERA_H_V_EXCHANGE          1
// #define CONFIG_VIDEO_720P
#ifndef CONFIG_VIDEO_720P
#define CONFIG_VIDEO_YUV_PPBUF_ENABLE
#endif
#define USER_ISC_STATIC_BUFF_ENABLE         1

#define TCFG_DEBUG_PORT                     IO_PORTB_03
#define TCFG_DAC_MUTE_PORT                  IO_PORTB_02
#define TCFG_DAC_MUTE_VALUE                 0
#define TCFG_DAC_MUTE_ENABLE                1

#define TCFG_CAMERA_XCLK_PORT               IO_PORTH_02
#define TCFG_CAMERA_RESET_PORT              IO_PORTH_03

#define TCFG_SW_IIC_CLK_PORT                IO_PORTH_00
#define TCFG_SW_IIC_DAT_PORT                IO_PORTH_01

#define TCFG_SD_PORTS                       'A'			//SD0/SD1的ABCD组(默认为开发板SD0-D),注意:IO占用问题
#define TCFG_SD_DAT_WIDTH                   1			//1:单线模式, 4:四线模式
#define TCFG_SD_DET_MODE                    SD_CLK_DECT	//检测模式:命令检测，时钟检测，IO检测
#define TCFG_SD_DET_IO                      IO_PORTB_08	//SD_DET_MODE为SD_IO_DECT时有效
#define TCFG_SD_DET_IO_LEVEL                0			//IO检卡上线的电平(0/1),SD_DET_MODE为SD_IO_DECT时有效
#if defined CONFIG_OVERCLOCKING_ENABLE
#define TCFG_SD_CLK                         33000000	//SD时钟
#else
#define TCFG_SD_CLK                         24000000	//SD时钟
#endif

//*********************************************************************************//
//                            AUDIO_ADC应用的通道配置                              //
//*********************************************************************************//
#define CONFIG_AUDIO_ENC_SAMPLE_SOURCE      AUDIO_ENC_SAMPLE_SOURCE_MIC
// #define CONFIG_AUDIO_ENC_SAMPLE_SOURCE      AUDIO_ENC_SAMPLE_SOURCE_PLNK0
#if CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_PLNK0
#define TCFG_MIC_CHANNEL_MAP                LADC_CH_MIC3_P_N
#define TCFG_MIC_CHANNEL_NUM                1
#else
#define TCFG_MIC_CHANNEL_MAP                (LADC_CH_MIC1_P_N | LADC_CH_MIC3_P_N)
#define TCFG_MIC_CHANNEL_NUM                2
#endif
#define TCFG_LINEIN_CHANNEL_MAP             (LADC_CH_AUX1 | LADC_CH_AUX3)
#define TCFG_LINEIN_CHANNEL_NUM             2

#define CONFIG_AISP_DIFFER_MIC_REPLACE_LINEIN       //用差分mic代替aec回采
#define CONFIG_ASR_CLOUD_ADC_CHANNEL        1		//云端识别mic通道
#define CONFIG_VOICE_NET_CFG_ADC_CHANNEL    1		//声波配网mic通道
#define CONFIG_AISP_MIC0_ADC_CHANNEL        1		//本地唤醒左mic通道
#define CONFIG_AISP_MIC_ADC_GAIN            100		//本地唤醒mic增益
#define CONFIG_AISP_LINEIN_ADC_CHANNEL      3		//本地唤醒LINEIN回采DAC通道
#define CONFIG_AISP_MIC1_ADC_CHANNEL        0		//本地唤醒右mic通道
#define CONFIG_REVERB_ADC_CHANNEL           1		//混响mic通道
#define CONFIG_PHONE_CALL_ADC_CHANNEL       1		//通话mic通道
#define CONFIG_UAC_MIC_ADC_CHANNEL          1		//UAC mic通道
#define CONFIG_AISP_LINEIN_ADC_GAIN         10		//本地唤醒LINEIN增益
#define CONFIG_AUDIO_LINEIN_CHANNEL         2		//LIENIN通道数
#define CONFIG_AUDIO_LINEIN_CHANNEL_MAP     TCFG_LINEIN_CHANNEL_MAP

//*********************************************************************************//
//                                        USB配置                                  //
//*********************************************************************************//
#define TCFG_PC_ENABLE                      1     //使用USB从机功能一定要打开
#if defined CONFIG_VIDEO_ENABLE
#define USB_DEVICE_CLASS_CONFIG             (UVC_CLASS | SPEAKER_CLASS | CDC_CLASS)
#define USB_DEVICE_CLASS_CONFIG_2_0         (MASSSTORAGE_CLASS | UVC_CLASS | HID_CLASS | AUDIO_CLASS)
#else
#define USB_DEVICE_CLASS_CONFIG             (SPEAKER_CLASS | CDC_CLASS)
#define USB_DEVICE_CLASS_CONFIG_2_0         (MASSSTORAGE_CLASS | HID_CLASS | AUDIO_CLASS)
#endif
#define TCFG_UDISK_ENABLE                   1     //U盘主机功能
#define TCFG_HOST_AUDIO_ENABLE              1     //uac主机功能，用户需要自己补充uac_host_demo.c里面的两个函数
#if defined CONFIG_VIDEO_ENABLE && !defined TCFG_HOST_UVC_ENABLE
#define TCFG_HOST_UVC_ENABLE                1     //UVC主机功能
#endif
#define TCFG_HID_HOST_ENABLE                1     //HID主机功能
#define TCFG_VIR_UDISK_ENABLE               0     //虚拟U盘
#if TCFG_HOST_UVC_ENABLE
#define CONFIG_UI_ENABLE
#define CONFIG_UVC_VIDEO2_ENABLE
#define CONFIG_VIDEO_DISPLAY_ENABLE
#endif

#ifdef CONFIG_PROJECT_MUTEX
#error "project config can not enable at the same time, just select one!!!"
#else
#define CONFIG_PROJECT_MUTEX
#endif

#endif	//CONFIG_BOARD_DEV_KIT

#endif
