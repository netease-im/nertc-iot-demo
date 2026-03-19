#ifndef _PRODUCT_MAIN_H_
#define _PRODUCT_MAIN_H_

#include "app_config.h"
#include "system/includes.h"
#include "json_c/json.h"
#include "json_c/json_object.h"
#include "json_c/json_tokener.h"
#include "usb_std_class_def.h"
#include "ioctl_cmds.h"
#include "wifi/wifi_connect.h"
#include "lcd_drive.h"
#include "camera.h"
#include "syscfg_id.h"
#include "gpio.h"
#include "fs/fs.h"
#include "lwipopts.h"  //包含对应的lwip配置文件
#include "event/device_event.h"
#include "event/key_event.h"

#include "datatype.h"
#include "product_port.h"
#include "communication.h"
#include "devices_control.h"
#include "product_net.h"


#ifdef PRODUCT_TEST_ENABLE

//*********************************************************************************//
//                                  PRODUCT测试相关配置                             //
//*********************************************************************************//
#define 	PRODUCT_TEST_DEBUG
//#define 	MSG_TEST_DEBUG

//#define		PRODUCT_SER_STATIC_IP_ENABLE
#ifdef PRODUCT_SER_STATIC_IP_ENABLE
#define		SER_IP				"192.168.0.12"
#define		SER_NETMASK			"255.255.255.0"
#define		SER_GATEWAY			"192.168.0.1"
#endif

#define 	AUDIO_CTL_PORT 		(33263)
#define 	AUDIO_DATA_PORT 	(33265)
#define 	CAMERA_PORT 		(37769)
#define 	HOST_PORT 			(32769)
#define 	BREATHE_PORT        (39769)
#define		BREATHE_TIMEOUT		(1000)
#define		PRODUCT_SER_AP_CH   (7)
#define		PRODUCT_CLI_RSSI_THR (-30)
#define		PRODUCT_SER_AP_PREFIX	"PRODUCT_SER_AP_"
#define		PRODUCT_SER_AP_PWD      "0123456789"
#define		PRODUCT_AP_PREFIX		"PRODUCT_AP_"
#define		PRODUCT_AP_PWD      	"9876543210"
#define		BREATHE_STR				"product_breathe"

#define 	DATA_BUFFER_ALIGN	(4)
#define 	TEST_BUFFER_SIZE	(sizeof(struct comm_head) + sizeof(u16) + 1024 * 1)
#define 	RESP_BUFFER_SIZE	(sizeof(struct comm_head) + sizeof(u16) + 1024 * 3)
#define 	RECV_BUFFER_SIZE	(sizeof(struct comm_head) + sizeof(u16) + 1024 * 3)

#define		PRODUCT_NET_CHECK_TIMEOUT	(0)
#define		PRODUCT_CAMERA_NUM			(2)
#define 	PRODUCT_UVC_FPS			    (5)
#define 	PRODUCT_MAC_SIZE			(6)
#define 	PRODUCT_UUID_SIZE			(32)
#define 	PRODUCT_SN_SIZE				(32)
#define 	PRODUCT_LICENSE_INFO_SIZE	(512)
#define 	PRODUCT_RTC_INFO_SIZE	    (sizeof(struct product_rtc_time))
#define 	PRODUCT_RES_SIZE	        (4)
#define 	PRODUCT_USER_DATA_SIZE	    (128)
#define 	PRODUCT_CFG_SIZE            (sizeof(struct product_conn))

#define 	IO_TRIGGER_MODE      (0) //"IO检测"方式触发进入测试模式
#define 	USER_DEF_MODE        (1) //"用户定义"方式触发进入测试模式
#define 	AUTO_STARTUP_MODE    (2) //上电进入测试模式



#define CONFIG_PD_TRIGGER_MODE   AUTO_STARTUP_MODE //触发方式配置

#define CONFIG_PD_TRIGGER_IO_PORT   IO_PORTC_01    //"IO检测"IO配置
#define CONFIG_PD_TRIGGER_IO_STATE  (1)        	   //"IO检测"IO的状态配置，0:低电平触发，1:高电平触发
#define CONFIG_PD_TRIGGER_IO_CNT    (10)           //"IO检测"IO的检测次数


#define DEV_STATUS_ON	(1)
#define DEV_STATUS_OFF	(0)
#define	DEV_MONITOR_PERIOD	(50) //ticks


//配置设备列表
#define _STR(x) #x
#define STR(x) _STR(x)
#define DEVICE_TYPE(id, dev) "{\"id\" : \""STR(id)"\", \"type\" : \""STR(dev)"\"}"

#define DEVICES_LIST "\"devs_list\" : [" \
	DEVICE_TYPE(0,DEV_TYPE_SD)","\
	DEVICE_TYPE(0,DEV_TYPE_LCD)","\
	DEVICE_TYPE(0,DEV_TYPE_MIC)","\
	DEVICE_TYPE(0,DEV_TYPE_BUGLE)","\
	DEVICE_TYPE(0,DEV_TYPE_CAMERA)","\
	DEVICE_TYPE(0,DEV_TYPE_BATTERY)","\
	DEVICE_TYPE(0,DEV_TYPE_WIFI)","\
	DEVICE_TYPE(0,DEV_TYPE_PIR)","\
	DEVICE_TYPE(0,DEV_TYPE_MOTOR)","\
	DEVICE_TYPE(0,DEV_TYPE_GSENSOR)","\
	DEVICE_TYPE(0,DEV_TYPE_TOUCHPANEL)","\
	DEVICE_TYPE(0,DEV_TYPE_KEYPAD)","\
	DEVICE_TYPE(0,DEV_TYPE_LED)\
	"]"


//配置固件信息
#define PD_TOOL_VERSION		("v1.0.0")
#define PD_CHIP_VERSION		("AC791x")
#define PD_SDK_VERSION		(sdk_version())


#ifdef PRODUCT_TEST_DEBUG
#define     product_info(x, ...)     printf("[PRODUCT_TEST][INFO] " x " ", ## __VA_ARGS__)
#define     product_err(x, ...)      printf("[PRODUCT_TEST][ERR] " x " ", ## __VA_ARGS__)
#else
#define     product_info(...)
#define     product_err(...)
#endif
//*********************************************************************************//
//                                  PRODUCT测试相关接口                            //
//*********************************************************************************//

/**
 * @brief 产测测试初始化
 *
 * @param
 *
 * @note
 */
u8 product_main(void);
u8 is_product_mode(void);
void product_online_tips(void);
void data_respond(u32 idx, u8 type, u8 *data, u32 len);
#endif //PRODUCT_TEST_ENABLE

#endif


