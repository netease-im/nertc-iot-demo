#ifndef CONFIG_BOARD_7916A_DEVELOP_CFG_H
#define CONFIG_BOARD_7916A_DEVELOP_CFG_H

#ifdef CONFIG_BOARD_DEVELOP

#define TCFG_SD0_ENABLE                     1
#define TCFG_ADKEY_ENABLE                   1          //AD按键
#if TCFG_ADKEY_ENABLE
#define CONFIG_PRESS_LONG_KEY_POWERON                  //长按开关机功能
#endif

#define TCFG_DEBUG_PORT                     IO_PORTC_06
#define TCFG_DAC_MUTE_PORT                  0xff
#define TCFG_DAC_MUTE_VALUE                 0
#define TCFG_DAC_MUTE_ENABLE                1

#define TCFG_CAMERA_XCLK_PORT               IO_PORTC_00
#define TCFG_CAMERA_RESET_PORT              -1

#define TCFG_SW_IIC_CLK_PORT                IO_PORTC_01
#define TCFG_SW_IIC_DAT_PORT                IO_PORTC_02

#define TCFG_SD_PORTS                       'D'			//SD0/SD1的ABCD组(默认为开发板SD0-D),注意:IO占用问题
#define TCFG_SD_DAT_WIDTH                   1			//1:单线模式, 4:四线模式
#define TCFG_SD_DET_MODE                    SD_CLK_DECT	//检测模式:命令检测，时钟检测，IO检测
#define TCFG_SD_DET_IO                      IO_PORTA_01	//SD_DET_MODE为SD_IO_DECT时有效
#define TCFG_SD_DET_IO_LEVEL                0			//IO检卡上线的电平(0/1),SD_DET_MODE为SD_IO_DECT时有效
#define TCFG_SD_CLK                         24000000	//SDIO时钟

//*********************************************************************************//
//                            AUDIO_ADC应用的通道配置                              //
//*********************************************************************************//
#define CONFIG_AUDIO_ENC_SAMPLE_SOURCE      AUDIO_ENC_SAMPLE_SOURCE_MIC
#define TCFG_MIC_CHANNEL_MAP                LADC_CH_MIC1_P_N
#define TCFG_MIC_CHANNEL_NUM                1
#define TCFG_LINEIN_CHANNEL_MAP             LADC_CH_AUX3
#define TCFG_LINEIN_CHANNEL_NUM             1

#define CONFIG_ASR_CLOUD_ADC_CHANNEL        1		//云端识别mic通道
#define CONFIG_VOICE_NET_CFG_ADC_CHANNEL    1		//声波配网mic通道
#define CONFIG_AISP_MIC0_ADC_CHANNEL        1		//本地唤醒左mic通道
#define CONFIG_AISP_MIC1_ADC_CHANNEL        3		//本地唤醒右mic通道
#define CONFIG_REVERB_ADC_CHANNEL           1		//混响mic通道
#define CONFIG_PHONE_CALL_ADC_CHANNEL       1		//通话mic通道
#define CONFIG_UAC_MIC_ADC_CHANNEL          1		//UAC mic通道
#define CONFIG_AISP_LINEIN_ADC_CHANNEL      3		//本地唤醒LINEIN回采DAC通道
#define CONFIG_AISP_MIC_ADC_GAIN            80		//本地唤醒mic增益
#define CONFIG_AISP_LINEIN_ADC_GAIN         60		//本地唤醒LINEIN增益
#define CONFIG_AUDIO_LINEIN_CHANNEL         1		//LIENIN通道数
#define CONFIG_AUDIO_LINEIN_CHANNEL_MAP     TCFG_LINEIN_CHANNEL_MAP

#ifdef CONFIG_PROJECT_MUTEX
#error "project config can not enable at the same time, just select one!!!"
#else
#define CONFIG_PROJECT_MUTEX
#endif

#endif	//CONFIG_BOARD_DEVELOP

#endif
