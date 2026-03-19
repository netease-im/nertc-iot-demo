#ifndef __RF_FCC_MAIN_H_
#define __RF_FCC_MAIN_H_

#include "app_config.h"
#include "generic/typedef.h"

#ifdef RF_FCC_TEST_ENABLE

#define IO_TRIGGER_MODE      (0) //"IO检测"方式触发进入RF_FCC测试模式
#define GPCNT_TRIGGER_MODE   (1) //"脉冲检测"方式触发进入RF_FCC测试模式
#define UART_CMD_MODE        (2) //"串口指令"方式触发进入RF_FCC测试模式
#define USER_DEF_MODE        (3) //"用户定义"方式触发进入RF_FCC测试模式
#define AUTO_STARTUP_MODE    (4) //上电进入RF_FCC测试模式


//*********************************************************************************//
//                                  FCC测试相关配置                                //
//*********************************************************************************//
// #define RF_FCC_IN_NORNAL_WORK    //使能在正常工作模式下进行RF_FCC测试
//#define RF_FCC_MODE_KEEP

#define CONFIG_RF_FCC_TRIGGER_MODE   AUTO_STARTUP_MODE //RF_FCC触发方式配置

#define CONFIG_RF_FCC_TRIGGER_IO_PORT   IO_PORTC_01    //RF_FCC"IO检测"IO配置
#define CONFIG_RF_FCC_TRIGGER_IO_STATE  (1)            //RF_FCC"IO检测"IO的状态配置，0:低电平触发，1:高电平触发
#define CONFIG_RF_FCC_TRIGGER_IO_CNT    (10)           //RF_FCC"IO检测"IO的检测次数

#define CONFIG_RF_FCC_GPCNT_TRIGGER_FREQ_H  (11000)    //RF_FCC"脉冲检测"频率上限，单位Hz
#define CONFIG_RF_FCC_GPCNT_TRIGGER_FREQ_L  (9000)     //RF_FCC"脉冲检测"频率下限，单位Hz
#define CONFIG_RF_FCC_GPCNT_TRIGGER_CNT     (10)       //RF_FCC"脉冲检测"次数
#define CONFIG_RF_FCC_TRIGGER_GPCNT_PORT    IO_PORTC_01//RF_FCC"脉冲检测"IO

#if (CONFIG_RF_FCC_TRIGGER_MODE == GPCNT_TRIGGER_MODE)
#define TCFG_GPCNT_ENABLE
#endif


//*********************************************************************************//
//                                  FCC相关接口说明                                //
//*********************************************************************************//


/**
 * @brief rf_fcc_test_init，RF_FCC测试初始化
 *
 * @param
 *
 * @note
 */
u8 rf_fcc_test_init(void);


/**
 * @brief wifi_tx_data_test，WIFI发送数据测试
 *
 * @param
 *          u8 channel     ：信道(1-13)
 *          u8 power       ：数字增益(0-128)
 *          u8 rate        ：速率
 *          u8 *packet     ：需发送的数据
 *          u32 packet_len ：需发送的数据长度(1-1513)
 *          u32 npackets   ：发送次数；设为0，则一直发送
 *          u32 tx_interval：发送间隔
 *
 * @note
 */
void wifi_tx_data_test(u8 channel, u8 power, u8 rate, u8 *packet, u32 packet_len, u32 npackets, u32 tx_interval);


/**
 * @brief rf_fcc_get_uart，获取RF_FCC测试的上位机通信串口号
 *
 * @param
 *
 * @note 可在外部定义同名函数，修改返回的通信串口号("uart0"/"uart1"/"uart2")
 */
__attribute__((weak)) const char *rf_fcc_get_uart(void);


/**
 * @brief fcc_enter_user_def，开机检测是否进入RF_FCC测试
 *
 * @param
 *
 * @note 可在外部定义同名函数，修改触发方式
 */
__attribute__((weak))u8 fcc_enter_user_def(void);


/**
 * @brief fcc_res_handler，RF_FCC测试结果处理
 *
 * @param res == true, 测试PASS
 *        res == false, 测试FAIL
 *
 * @note 可在外部定义同名函数，根据测试结果添加自定义操作
 */
__attribute__((weak))void fcc_res_handler(u8 res);

void fcc_bt_ble_module_init(void);

#endif //RF_FCC_TEST_ENABLE

#endif


