#ifndef __RCSP_CFG_H__
#define __RCSP_CFG_H__

#include "rcsp_define.h"
#include "app_config.h"

// 全局搜索 RCSP_MODE 查找当前SDK相关配置
/*
JL_EARPHONE_APP_EN
#if (SOUNDCARD_ENABLE)
RCSP_MODE == RCSP_MODE_SOUNDBOX
#if ((RCSP_MODE == RCSP_MODE_EARPHONE) || (RCSP_MODE == RCSP_MODE_EARPHONE_V2022))
#if (RCSP_MODE == RCSP_MODE_WATCH)
#if (RCSP_MODE == RCSP_MODE_EARPHONE_V2022)
*/

#define ANCS_CLIENT_EN							0
#define DUEROS_DMA_EN 			 				0
#define JL_RCSP_NFC_DATA_OPT				0		// NFC数据传输


#if (!defined(RCSP_MODE) || (RCSP_MODE == 0))

#define OTA_TWS_SAME_TIME_ENABLE     			0
#define RCSP_UPDATE_EN               			0
#define UPDATE_MD5_ENABLE            			0     //升级是否支持MD5校验
#define RCSP_FILE_OPT				 			0
#define JL_EARPHONE_APP_EN			 			0
#define TCFG_BS_DEV_PATH_EN			 			0
#define WATCH_FILE_TO_FLASH          			0
#define UPDATE_EX_FALSH_USE_4K_BUF   			0

#else // (!defined(RCSP_MODE) || (RCSP_MODE == 0))


// 音箱SDK RCSP功能配置
#if (RCSP_MODE == RCSP_MODE_SOUNDBOX)

#define JL_EARPHONE_APP_EN			 			0
#define RCSP_ADV_EN					1

#define RCSP_UPDATE_EN		         			0     //是否支持rcsp升级
#define OTA_TWS_SAME_TIME_ENABLE     			0     //是否支持TWS同步升级
#define UPDATE_MD5_ENABLE            			1     //升级是否支持MD5校验
#define RCSP_FILE_OPT				 			1
#define TCFG_BS_DEV_PATH_EN			 			1



// 默认的功能模块使能
// 可在板级配置头文件中定义客户定制的功能
#ifndef JL_RCSP_CUSTOM_APP_EN
#define RCSP_ADV_NAME_SET_ENABLE        		1
#define RCSP_ADV_KEY_SET_ENABLE         		0
#define RCSP_ADV_LED_SET_ENABLE         		0
#define RCSP_ADV_MIC_SET_ENABLE         		0
#define RCSP_ADV_WORK_SET_ENABLE        		0
#define RCSP_ADV_EQ_SET_ENABLE          		0
#define RCSP_ADV_MUSIC_INFO_ENABLE      		0
#define RCSP_ADV_HIGH_LOW_SET					0
#define RCSP_ADV_PRODUCT_MSG_ENABLE     		1
#define RCSP_ADV_FIND_DEVICE_ENABLE     		1
#define RCSP_ADV_COLOR_LED_SET_ENABLE   		0
#define RCSP_ADV_KARAOKE_SET_ENABLE				0
#define RCSP_ADV_KARAOKE_EQ_SET_ENABLE			0
#endif



// 手表SDK RCSP功能配置
#elif (RCSP_MODE == RCSP_MODE_WATCH)

#define JL_EARPHONE_APP_EN			 			0
#define RCSP_ADV_EN					0

#define JL_RCSP_SENSORS_DATA_OPT			0		// 传感器功能

//BLE多连接,多开注意RAM的使用--中强添加
#define RCSP_MULTI_BLE_EN                  	0 //蓝牙BLE多连:1主1从
#define RCSP_MULTI_BLE_SLAVE_NUMS          	1 //range(0~1)
#define RCSP_MULTI_BLE_MASTER_NUMS         	1 //range(0~2)
#define CONFIG_BT_GATT_CONNECTION_NUM       	RCSP_MULTI_BLE_SLAVE_NUMS + RCSP_MULTI_BLE_MASTER_NUMS

// #define CONFIG_APP_OTA_ENABLE               	1     //是否支持RCSP升级(JL-OTA)
#define RCSP_UPDATE_EN		         			0     //是否支持rcsp升级
#define OTA_TWS_SAME_TIME_ENABLE     			0     //是否支持TWS同步升级
#define UPDATE_MD5_ENABLE            			0     //升级是否支持MD5校验
#define RCSP_FILE_OPT				 			1
#define TCFG_BS_DEV_PATH_EN			 			1
#define WATCH_FILE_TO_FLASH          			1
#define UPDATE_EX_FALSH_USE_4K_BUF   			1

#define	JL_RCSP_SIMPLE_TRANSFER 			1

#define JL_RCSP_EXTRA_FLASH_OPT			0

#if TCFG_APP_RTC_EN
// 0 - 旧闹钟，只支持提示音闹铃
// 1 - 新闹钟，可选择提示音或者设备音乐作为闹铃
#define CUR_RTC_ALARM_MODE						1
#endif

// 默认的功能模块使能
// 可在板级配置头文件中定义客户定制的功能
#ifndef JL_RCSP_CUSTOM_APP_EN
#define RCSP_ADV_NAME_SET_ENABLE        1
#define RCSP_ADV_KEY_SET_ENABLE         0
#define RCSP_ADV_LED_SET_ENABLE         0
#define RCSP_ADV_MIC_SET_ENABLE         0
#define RCSP_ADV_WORK_SET_ENABLE        0
#define RCSP_ADV_EQ_SET_ENABLE          0
#define RCSP_ADV_MUSIC_INFO_ENABLE      0
#define RCSP_ADV_HIGH_LOW_SET			0
#define RCSP_ADV_PRODUCT_MSG_ENABLE     0
#define RCSP_ADV_FIND_DEVICE_ENABLE     1
#define RCSP_ADV_COLOR_LED_SET_ENABLE   0
#define RCSP_ADV_KARAOKE_SET_ENABLE		0
#define RCSP_ADV_KARAOKE_EQ_SET_ENABLE	0
#endif




// 耳机SDK RCSP功能配置
#elif (RCSP_MODE == RCSP_MODE_EARPHONE)

// TODO
#define JL_EARPHONE_APP_EN			 			1
#define RCSP_ADV_EN					1





// 耳机SDK_V2022 RCSP功能配置
#elif (RCSP_MODE == RCSP_MODE_EARPHONE_V2022)

// TODO
#define JL_EARPHONE_APP_EN			 			1
#define RCSP_ADV_EN					1



// 通用 RCSP功能配置
#else // (RCSP_MODE == RCSP_MODE_COMMON)

// TODO
#define JL_EARPHONE_APP_EN			 			0
#define RCSP_ADV_EN					1







#endif // RCSP_MODE

#if JL_EARPHONE_APP_EN

// 默认的功能模块使能
// 可在板级配置头文件中定义客户定制的功能
#ifndef JL_RCSP_CUSTOM_APP_EN
#define RCSP_ADV_NAME_SET_ENABLE        1
#define RCSP_ADV_KEY_SET_ENABLE         0
#define RCSP_ADV_LED_SET_ENABLE         0//1
#define RCSP_ADV_MIC_SET_ENABLE         0
#define RCSP_ADV_WORK_SET_ENABLE        0
#define RCSP_ADV_EQ_SET_ENABLE          1
#define RCSP_ADV_MUSIC_INFO_ENABLE      1
#define RCSP_ADV_HIGH_LOW_SET			1
#define RCSP_ADV_PRODUCT_MSG_ENABLE     1
#define RCSP_ADV_FIND_DEVICE_ENABLE     1
#define RCSP_ADV_COLOR_LED_SET_ENABLE   0
#define RCSP_ADV_KARAOKE_SET_ENABLE		1
#define RCSP_ADV_KARAOKE_EQ_SET_ENABLE	1
#define RCSP_ADV_ANC_VOICE     			1
#define RCSP_ADV_ASSISTED_HEARING		0

#endif




#endif // JL_EARPHONE_APP_EN


#if   (defined CONFIG_CPU_BR21)
#define		RCSP_SDK_TYPE		RCSP_SDK_TYPE_AC692X
#elif (defined CONFIG_CPU_BR22)
#define		RCSP_SDK_TYPE		RCSP_SDK_TYPE_AC693X
#elif (defined CONFIG_CPU_BR23 && SOUNDCARD_ENABLE)
#define     RCSP_SDK_TYPE       RCSP_SDK_TYPE_AC695X_SOUNDCARD
#elif (defined CONFIG_CPU_BR23 && (RCSP_MODE == RCSP_MODE_WATCH))
#define		RCSP_SDK_TYPE		RCSP_SDK_TYPE_AC695X_WATCH
#elif (defined CONFIG_CPU_BR23)
#define		RCSP_SDK_TYPE		RCSP_SDK_TYPE_AC695X
#elif (defined CONFIG_CPU_BR25)
#define		RCSP_SDK_TYPE		RCSP_SDK_TYPE_AC696X
#elif (defined CONFIG_CPU_BR28)
#if (RCSP_MODE == RCSP_MODE_WATCH)
#define		RCSP_SDK_TYPE		RCSP_SDK_TYPE_AC701N_WATCH
#else
#define		RCSP_SDK_TYPE		RCSP_SDK_TYPE_AC696X
#endif
#elif (defined CONFIG_CPU_BR30)
#define		RCSP_SDK_TYPE		RCSP_SDK_TYPE_AC697X
#else
#define		RCSP_SDK_TYPE		RCSP_SDK_TYPE_AC693X
#endif


// 默认RCSP配置，如果没有定义，则默认为0
#ifndef RCSP_BTMATE_EN
#define RCSP_BTMATE_EN      0
#endif

#ifndef RCSP_ADV_EN
#define RCSP_ADV_EN         0
#endif

#ifndef RCSP_ADV_NAME_SET_ENABLE
#define RCSP_ADV_NAME_SET_ENABLE                0
#endif

#ifndef RCSP_ADV_KEY_SET_ENABLE
#define RCSP_ADV_KEY_SET_ENABLE                 0
#endif

#ifndef RCSP_ADV_LED_SET_ENABLE
#define RCSP_ADV_LED_SET_ENABLE                 0
#endif

#ifndef RCSP_ADV_MIC_SET_ENABLE
#define RCSP_ADV_MIC_SET_ENABLE                 0
#endif

#ifndef RCSP_ADV_WORK_SET_ENABLE
#define RCSP_ADV_WORK_SET_ENABLE                0
#endif

#ifndef RCSP_ADV_HIGH_LOW_SET
#define RCSP_ADV_HIGH_LOW_SET                   0
#endif

#ifndef RCSP_ADV_MUSIC_INFO_ENABLE
#define RCSP_ADV_MUSIC_INFO_ENABLE              0
#endif

#ifndef RCSP_ADV_KARAOKE_SET_ENABLE
#define RCSP_ADV_KARAOKE_SET_ENABLE             0
#endif

#ifndef RCSP_ADV_PRODUCT_MSG_ENABLE
#define RCSP_ADV_PRODUCT_MSG_ENABLE        		0
#endif

#ifndef RCSP_ADV_COLOR_LED_SET_ENABLE
#define RCSP_ADV_COLOR_LED_SET_ENABLE   		0
#endif

#ifndef RCSP_ADV_KARAOKE_EQ_SET_ENABLE
#define RCSP_ADV_KARAOKE_EQ_SET_ENABLE			0
#endif

#ifndef RCSP_ADV_EQ_SET_ENABLE
#define RCSP_ADV_EQ_SET_ENABLE          		0
#endif

#ifndef RCSP_ADV_FIND_DEVICE_ENABLE
#define RCSP_ADV_FIND_DEVICE_ENABLE				0
#endif

#ifndef RCSP_FILE_OPT
#define RCSP_FILE_OPT       0
#endif

#ifndef RCSP_UPDATE_EN
#define RCSP_UPDATE_EN                  0
#endif




#endif // (!defined(RCSP_MODE) || (RCSP_MODE == 0))




#endif // __RCSP_CFG_H__

