#ifndef CONFIG_BOARD_7916A_DEMO_CFG_H
#define CONFIG_BOARD_7916A_DEMO_CFG_H

#ifdef CONFIG_BOARD_DEMO	//测试DEMO板

#define TCFG_SD0_ENABLE                     1
#define TCFG_ADKEY_ENABLE                   1           //AD按键
#if TCFG_ADKEY_ENABLE
#define CONFIG_PRESS_LONG_KEY_POWERON                   //长按开关机功能
#endif
#define CONFIG_CAMERA_H_V_EXCHANGE          1

#define TCFG_DEBUG_PORT                     IO_PORTB_08
#define TCFG_DAC_MUTE_PORT                  0xff
#define TCFG_DAC_MUTE_VALUE                 0

#define TCFG_CAMERA_XCLK_PORT               0xff
#define TCFG_CAMERA_RESET_PORT              0xff

#define TCFG_SW_IIC_CLK_PORT                IO_PORTH_00
#define TCFG_SW_IIC_DAT_PORT                IO_PORTH_01

#define TCFG_SD_PORTS                       'A'			//SD0/SD1的ABCD组(默认为开发板SD0-D),注意:IO占用问题
#define TCFG_SD_DAT_WIDTH                   4			//1:单线模式, 4:四线模式
#define TCFG_SD_DET_MODE                    SD_CLK_DECT	//检测模式:命令检测，时钟检测，IO检测
#define TCFG_SD_DET_IO                      IO_PORTB_08	//SD_DET_MODE为SD_IO_DECT时有效
#define TCFG_SD_DET_IO_LEVEL                0			//IO检卡上线的电平(0/1),SD_DET_MODE为SD_IO_DECT时有效
#define TCFG_SD_CLK                         24000000	//SD时钟

//*********************************************************************************//
//                            AUDIO_ADC应用的通道配置                              //
//*********************************************************************************//
#define CONFIG_AUDIO_ENC_SAMPLE_SOURCE      AUDIO_ENC_SAMPLE_SOURCE_MIC
// #define CONFIG_AUDIO_ENC_SAMPLE_SOURCE      AUDIO_ENC_SAMPLE_SOURCE_PLNK0
#if CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_PLNK0
#define TCFG_MIC_CHANNEL_MAP                LADC_CH_MIC3_P_N
#define TCFG_MIC_CHANNEL_NUM                1
#else
#define TCFG_MIC_CHANNEL_MAP                (LADC_CH_MIC0_P_N | LADC_CH_MIC1_P_N | LADC_CH_MIC3_P_N)
#define TCFG_MIC_CHANNEL_NUM                3
#endif
#define TCFG_LINEIN_CHANNEL_MAP             (LADC_CH_AUX1 | LADC_CH_AUX3)
#define TCFG_LINEIN_CHANNEL_NUM             2

#define CONFIG_AISP_DIFFER_MIC_REPLACE_LINEIN       //用差分mic代替aec回采
#define CONFIG_ASR_CLOUD_ADC_CHANNEL        1		//云端识别mic通道
#define CONFIG_VOICE_NET_CFG_ADC_CHANNEL    1		//声波配网mic通道
#define CONFIG_AISP_MIC0_ADC_CHANNEL        1		//本地唤醒左mic通道
#define CONFIG_AISP_MIC_ADC_GAIN            80		//本地唤醒mic增益
#define CONFIG_AISP_LINEIN_ADC_CHANNEL      3		//本地唤醒LINEIN回采DAC通道
#define CONFIG_AISP_MIC1_ADC_CHANNEL        0		//本地唤醒右mic通道
#define CONFIG_REVERB_ADC_CHANNEL           1		//混响mic通道
#define CONFIG_PHONE_CALL_ADC_CHANNEL       1		//通话mic通道
#define CONFIG_UAC_MIC_ADC_CHANNEL          1		//UAC mic通道
#define CONFIG_AISP_LINEIN_ADC_GAIN         10		//本地唤醒LINEIN增益
#define CONFIG_AUDIO_LINEIN_CHANNEL         2		//LIENIN通道数
#define CONFIG_AUDIO_LINEIN_CHANNEL_MAP     TCFG_LINEIN_CHANNEL_MAP

#ifdef CONFIG_PROJECT_MUTEX
#error "project config can not enable at the same time, just select one!!!"
#else
#define CONFIG_PROJECT_MUTEX
#endif

#endif	//CONFIG_BOARD_DEMO

#endif
