#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#define AUDIO_ENC_SAMPLE_SOURCE_MIC         0
#define AUDIO_ENC_SAMPLE_SOURCE_PLNK0       1
#define AUDIO_ENC_SAMPLE_SOURCE_PLNK1       2
#define AUDIO_ENC_SAMPLE_SOURCE_IIS0        3
#define AUDIO_ENC_SAMPLE_SOURCE_IIS1        4
#define AUDIO_ENC_SAMPLE_SOURCE_LINEIN      5

#define CONFIG_AUDIO_DEC_PLAY_SOURCE        "dac"

#include "board_config.h"

// #define CONFIG_SDFILE_EXT_ENABLE         //外挂隐藏sdfile区的支持
// #define CONFIG_DMSDX_ENABLE              //msd多分区显示支持

#ifdef CONFIG_DMSDX_ENABLE

#ifdef CONFIG_SDFILE_EXT_ENABLE
#undef CONFIG_SFC_ENABLE
#undef __FLASH_SIZE__
#define __FLASH_SIZE__    (16 * 1024 * 1024)
#endif

#define CONFIG_SDNAND_HFS_LEN_TEXT  16M
#define CONFIG_SDNAND_HFAT_LEN_TEXT 4G
#define CONFIG_SDNAND_FAT1_LEN_TEXT 8G
#define CONFIG_SDNAND_HFS_LEN       (16llu * 1024 * 1024)
#define CONFIG_SDNAND_HFAT_LEN      (4llu * 1024 * 1024 * 1024)
#define CONFIG_SDNAND_FAT1_LEN      (8llu * 1024 * 1024 * 1024)

//每个簇有多少个block，比如簇大小设置为32768时，该值为32768 / 512 = 0x40
#if CONFIG_SDNAND_HFAT_LEN > 0x80000000
#define CONFIG_SDNAND_HFAT_CLUSTER_SIZE 0x40
#elif CONFIG_SDNAND_HFAT_LEN > 0x40000000
#define CONFIG_SDNAND_HFAT_CLUSTER_SIZE 0x20
#elif CONFIG_SDNAND_HFAT_LEN > 0x20000000
#define CONFIG_SDNAND_HFAT_CLUSTER_SIZE 0x10
#elif CONFIG_SDNAND_HFAT_LEN > 0x10000000
#define CONFIG_SDNAND_HFAT_CLUSTER_SIZE 0x8
#elif CONFIG_SDNAND_HFAT_LEN > 0x8000000
#define CONFIG_SDNAND_HFAT_CLUSTER_SIZE 0x4
#elif CONFIG_SDNAND_HFAT_LEN > 0x4000000
#define CONFIG_SDNAND_HFAT_CLUSTER_SIZE 0x2
#else
#define CONFIG_SDNAND_HFAT_CLUSTER_SIZE 0x1
#endif

#if CONFIG_SDNAND_FAT1_LEN > 0x80000000
#define CONFIG_SDNAND_FAT1_CLUSTER_SIZE 0x40
#elif CONFIG_SDNAND_FAT1_LEN > 0x40000000
#define CONFIG_SDNAND_FAT1_CLUSTER_SIZE 0x20
#elif CONFIG_SDNAND_FAT1_LEN > 0x20000000
#define CONFIG_SDNAND_FAT1_CLUSTER_SIZE 0x10
#elif CONFIG_SDNAND_FAT1_LEN > 0x10000000
#define CONFIG_SDNAND_FAT1_CLUSTER_SIZE 0x8
#elif CONFIG_SDNAND_FAT1_LEN > 0x8000000
#define CONFIG_SDNAND_FAT1_CLUSTER_SIZE 0x4
#elif CONFIG_SDNAND_FAT1_LEN > 0x4000000
#define CONFIG_SDNAND_FAT1_CLUSTER_SIZE 0x2
#else
#define CONFIG_SDNAND_FAT1_CLUSTER_SIZE 0x1
#endif

#endif

//*********************************************************************************//
//                          预留区UI和AUDIO资源配置                                //
//*********************************************************************************//
//注意：ui和audio资源的起始地址和大小, 根据产品生命周期最大情况定义,根据实际需求进行配置
//FLASH后面4K用于一些配置存储，所以禁止覆盖
//具体说明和注意事项请阅读文档

//(1)ui和audio资源 ,如果存在ui资源则位于扩展预留区末尾,
//还存在audio则位于ui项前面，其他配置项则位于它们之前.
/*
#------------------------------|
#  (其他预留区配置项)          |
#------------------------------|<----CONFIG_UI_PACKRES_ADR - CONFIG_AUDIO_PACKRES_LEN = CONFIG_AUDIO_PACKRES_ADR
#  (CONFIG_AUDIO_PACKRES_LEN)  |
#------------------------------|<----__FLASH_SIZE__ - 0x1000 - CONFIG_UI_PACKRES_LEN = CONFIG_UI_PACKRES_ADR
#  (CONFIG_UI_PACKRES_LEN)     |
#------------------------------|<----__FLASH_SIZE__ - 0x1000
#  (4K Reserved)               |
#------------------------------+<----__FLASH_SIZE__
*/

//(2)只有audio资源,则将其位于扩展预留区末尾.
/*
#------------------------------|
#  (其他预留区配置项)          |
#------------------------------|<----__FLASH_SIZE__ - 0x1000 - CONFIG_AUDIO_PACKRES_LEN = CONFIG_AUDIO_PACKRES_ADR
#  (CONFIG_AUDIO_PACKRES_LEN)  |
#------------------------------|<----__FLASH_SIZE__ - 0x1000
#  (4K Reserved)               |
#------------------------------+<----__FLASH_SIZE__
*/

#if defined CONFIG_UI_ENABLE && !defined CONFIG_SDFILE_EXT_ENABLE
#define CONFIG_UI_FILE_SAVE_IN_RESERVED_EXPAND_ZONE //UI资源打包后放在扩展预留区
#endif

#if defined CONFIG_AUDIO_ENABLE && !defined CONFIG_SDFILE_EXT_ENABLE
#define CONFIG_VOICE_PROMPT_FILE_SAVE_IN_RESERVED_EXPAND_ZONE //AUDIO资源打包后放在扩展预留区
#endif

#if defined CONFIG_UI_FILE_SAVE_IN_RESERVED_EXPAND_ZONE
#define CONFIG_UI_PACKRES_LEN 0x180000
#define CONFIG_UI_PACKRES_ADR ((__FLASH_SIZE__) - (CONFIG_UI_PACKRES_LEN) - 0x1000)
#else
#define CONFIG_UI_PACKRES_LEN 0
#define CONFIG_UI_PACKRES_ADR 0
#endif

#if defined CONFIG_VOICE_PROMPT_FILE_SAVE_IN_RESERVED_EXPAND_ZONE
#if defined CONFIG_UI_FILE_SAVE_IN_RESERVED_EXPAND_ZONE
#define CONFIG_AUDIO_PACKRES_LEN 0x180000
#define CONFIG_AUDIO_PACKRES_ADR ((__FLASH_SIZE__) - (CONFIG_UI_PACKRES_LEN) - 0x1000 - CONFIG_AUDIO_PACKRES_LEN)
#else
#define CONFIG_AUDIO_PACKRES_LEN 0x180000
#define CONFIG_AUDIO_PACKRES_ADR ((__FLASH_SIZE__) - CONFIG_AUDIO_PACKRES_LEN - 0x1000)
#endif
#else
#define CONFIG_AUDIO_PACKRES_LEN 0
#define CONFIG_AUDIO_PACKRES_ADR 0
#endif


#if !defined CONFIG_VIDEO_ENABLE || defined CONFIG_NO_SDRAM_ENABLE
#undef  CONFIG_RTOS_AND_MM_LIB_CODE_SECTION_IN_SDRAM
#endif

#ifdef CONFIG_NO_SDRAM_ENABLE
#undef  __SDRAM_SIZE__
#define __SDRAM_SIZE__    (0 * 1024 * 1024)
#endif

#if defined __SDRAM_SIZE__ && (__SDRAM_SIZE__ == 0)
#define CONFIG_NO_SDRAM_ENABLE        1
#endif

#ifndef TCFG_ADKEY_ENABLE
#define TCFG_ADKEY_ENABLE             0         //AD按键
#endif

#ifndef TCFG_IOKEY_ENABLE
#define TCFG_IOKEY_ENABLE             0         //IO按键
#define TCFG_IO_MULTIPLEX_WITH_SD     0
#endif

#ifndef TCFG_IRKEY_ENABLE
#define TCFG_IRKEY_ENABLE             0         //红外遥控按键
#endif

#ifndef TCFG_RDEC_KEY_ENABLE
#define TCFG_RDEC_KEY_ENABLE          0         //旋转编码器
#endif

#ifndef TCFG_TOUCH_KEY_ENABLE
#define TCFG_TOUCH_KEY_ENABLE         0         //PLCNT触摸按键
#endif

#ifndef TCFG_CTMU_TOUCH_KEY_ENABLE
#define TCFG_CTMU_TOUCH_KEY_ENABLE    0         //CTMU触摸按键
#endif

#ifndef TCFG_SD0_ENABLE
#define TCFG_SD0_ENABLE         0
#endif

#ifndef TCFG_SD1_ENABLE
#define TCFG_SD1_ENABLE         0
#endif

#define CONFIG_RTC_ENABLE                       //RTC开关
#define CONFIG_DEBUG_ENABLE                     //打印开关
#define RTOS_STACK_CHECK_ENABLE                 //是否启用定时检查任务栈
#ifdef CONFIG_RELEASE_ENABLE
// #define CONFIG_EXCEPTION_AUTO_FIX_ENABLE     //异常自动增大电源档位配置和降频
#endif
// #define CONFIG_SAVE_EXCEPTION_LOG_IN_FLASH   //保存异常打印信息到flash
// #define MEM_LEAK_CHECK_ENABLE                //是否启用内存泄漏检查(需要包含mem_leak_test.h头文件)
// #define CONFIG_AUTO_SHUTDOWN_ENABLE          //自动倒数关机
// #define CONFIG_SYS_VDD_CLOCK_ENABLE          //系统可使用动态电源、时钟配置
// #define CONFIG_IPMASK_ENABLE                 //系统使用不可屏蔽中断
// #define CONFIG_INTERNAL_VDDIO_POWER_SUPPLY_ENABLE        //实际PCB设计如果采用内部VDDIO供电，必须定义该宏

//*********************************************************************************//
//                                  FCC测试相关配置                                //
//*********************************************************************************//
//#define RF_FCC_TEST_ENABLE//使能RF_FCC测试，详细配置见"apps/common/rf_fcc_tool/include/rf_fcc_main.h"

//*********************************************************************************//
//                                  网络相关配置                                   //
//*********************************************************************************//
#ifdef CONFIG_NET_ENABLE
// #define CONFIG_LTE_PHY_ENABLE                //usb网卡
#define CONFIG_WIFI_ENABLE  					/*无线WIFI*/
#ifdef CONFIG_NO_SDRAM_ENABLE
#define CONFIG_RF_TRIM_CODE_MOVABLE //把RF TRIM 的运行代码动态加载到ram运行(节省4K ram内存), 防止RF TRIM 期间500ms大电流访问flash造成flash挂掉持续大电流
#else
#define CONFIG_RF_TRIM_CODE_AT_RAM //把RF TRIM 的运行代码定死到ram运行(浪费4K ram内存,否则若动态加载到sdram需清cache), 防止RF TRIM 期间500ms大电流访问flash造成flash挂掉持续大电流
#endif
// #define CONFIG_IPERF_ENABLE  				// iperf测试
// #define CONFIG_WIFIBOX_ENABLE                //WIFI测试盒性能测试
// #define CONFIG_MP_TEST_ENABLE                //RF量产测试
// #define CONFIG_READ_RF_PARAM_FROM_CFGTOOL_ENABLE    //从配置文件读取RF参数
//#define CONFIG_SERVER_ASSIGN_PROFILE		//第三方平台的profile由杰理服务器分配
// #define CONFIG_PROFILE_UPDATE				//每次开机都从杰理服务器获取第三方平台的profile,不读flash
// #define CONFIG_STATIC_IPADDR_ENABLE          //记忆路由器分配的IP,下次直接使用记忆IP节省DHCP时间
//#define CONFIG_ASSIGN_MACADDR_ENABLE        //第一次开机连上外网后，使用杰理服务器分配WIFI模块的MAC地址, 关闭则使用<蓝牙地址更新工具*.exe>或者随机数作为MAC地址
// #define CONFIG_TURING_SDK_ENABLE             //使用图灵平台
// #define CONFIG_DEEPBRAIN_SDK_ENABLE			//使用义语平台
// #define CONFIG_DUER_SDK_ENABLE				//使用百度平台
// #define CONFIG_ECHO_CLOUD_SDK_ENABLE			//使用回声云平台
// #define CONFIG_DUI_SDK_ENABLE	            //使用思必驰DUI平台
// #define CONFIG_ALI_SDK_ENABLE	            //使用天猫精灵平台
// #define CONFIG_TVS_SDK_ENABLE	            //使用腾讯云平台
// #define CONFIG_TELECOM_SDK_ENABLE            //电信云平台
// #define CONFIG_JL_CLOUD_SDK_ENABLE           //打开使用杰理云平台
// #define CONFIG_DLNA_SDK_ENABLE               //打开DLNA音乐播放功能
// #define CONFIG_DOWNLOAD_SAVE_FILE            //保存网络下载的音乐文件
// #define PAYMENT_AUDIO_SDK_ENABLE				//打开语音播报功能
// #define CONFIG_SCAN_PEN_ENABLE               //扫描笔图像拼接
// #define CONFIG_HTTP_SERVER_ENABLE            //HTTP服务器
// #define CONFIG_FTP_SERVER_ENABLE             //FTP服务器
// #define CONFIG_VOLC_LLM_ENABLE               //火山大模型
// #define CONFIG_ONESDK_LLM_ENABLE
// #define CONFIG_SXY_QYAI_ENABLE
// #define CONFIG_DUER_LC_DEMO_ENABLE           //小度澜川AI对话DEMO
// #define CONFIG_TWETALK_ENABLE
#define CONFIG_GIZWITS_SDK_ENABLE
#define CONFIG_NERTC_CONNECTION               //使能NERTC音视频通话连接(与Coze WebSocket互斥)

/*************电信云平台配网方式选择*************/
#ifdef CONFIG_TELECOM_SDK_ENABLE
#define CONFIG_APLINK_NET_CFG                 //电信AP配网。注意：1.电信AP配网不能与elink无感配网同时使能。2.wifi库需要支持ap
#ifndef CONFIG_APLINK_NET_CFG
#define CONFIG_ELINK_QLINK_NET_CFG            //电信elink无感配网。注意：elink无感配网不能与AIRKISS配网同时使能。
#endif
#define CONFIG_CTEI_DEVICE_ENABLE             //电信非蜂窝类串码对接设备使用
//#define CONFIG_MC_DEVICE_ENABLE               //电信mc接口类直连设备使用
#endif

#ifdef CONFIG_VIDEO_ENABLE
/*************绘本识别服务器*************/
#ifdef CONFIG_TURING_SDK_ENABLE
#define CONFIG_TURING_PAGE_TURNING_DET_ENABLE	//图灵翻页算法，需要配合图灵绘本平台使用
#endif

// #define CONFIG_WT_SDK_ENABLE                 //新玩瞳接口
#define CONFIG_QR_CODE_NET_CFG					//二维码配网
#endif

#ifdef CONFIG_WIFI_ENABLE
#define CONFIG_AIRKISS_NET_CFG					//AIRKISS配网
// #define CONFIG_WSC_NET_CFG					//WSC配网
#endif

#ifdef CONFIG_AUDIO_ENABLE
//#define CONFIG_ACOUSTIC_COMMUNICATION_ENABLE	//声波配网
#ifndef CONFIG_NO_SDRAM_ENABLE

#define AISP_ALGORITHM 		1 //思必驰双mic唤醒,未授权版本只支持运行10分钟
#define ROOBO_ALGORITHM 	2 //ROOBO 单/双MIC唤醒 ROOBO_DUAL_MIC_ALGORITHM ,,,测试版本只支持运行10分钟
#define WANSON_ALGORITHM 	3 //华镇算法,测试版只能够正确识别1000次
#define JLKWS_ALGORITHM 	4 //杰理算法,现阶段仅用于测试效果
//#define CONFIG_ASR_ALGORITHM  AISP_ALGORITHM    //本地打断唤醒算法选择

#ifdef CONFIG_ASR_ALGORITHM
//#define WIFI_PCM_STREAN_SOCKET_ENABLE           //打开打断唤醒pcm音频流局域网传输工具
#endif
#endif
#endif

#endif

//*********************************************************************************//
//                                  AUDIO配置                                      //
//*********************************************************************************//
#ifdef CONFIG_AUDIO_ENABLE

// #define CONFIG_DEC_DIGITAL_VOLUME_ENABLE     //数字音量淡入淡出功能
// #define CONFIG_DEC_ANALOG_VOLUME_ENABLE      //模拟音量淡入淡出功能
// #define CONFIG_RESUME_LOCAL_PLAY_FILE        //本地断点恢复播放
#if (!defined CONFIG_SFC_ENABLE) && (defined __SDRAM_SIZE__ && __SDRAM_SIZE__ >= (8 * 1024 * 1024))
// #define LOAD_SDRAM_PROMPT_PLAY 			0		//0 首次调用提示音装载全部进sdram  1 装载当前提示音
#endif

#ifdef CONFIG_BT_ENABLE
#define CONFIG_SBC_DEC_ENABLE
#if __FLASH_SIZE__ > (1 * 1024 * 1024)
#define CONFIG_SBC_ENC_ENABLE
#define CONFIG_MSBC_DEC_ENABLE
#define CONFIG_MSBC_ENC_ENABLE
#define CONFIG_CVSD_DEC_ENABLE
#define CONFIG_CVSD_ENC_ENABLE
#endif
#endif

#define CONFIG_MIDI_CTRL_DEC_ENABLE
#define CONFIG_PCM_DEC_ENABLE
#define CONFIG_PCM_ENC_ENABLE
#define CONFIG_WAV_DEC_ENABLE
#define CONFIG_WAV_ENC_ENABLE
#if __FLASH_SIZE__ > (1 * 1024 * 1024)
#define CONFIG_MP3_DEC_ENABLE
#define CONFIG_M4A_DEC_ENABLE
// #define CONFIG_NEW_M4A_DEC_ENABLE	//需要把lib_m4a_dec.a替换成lib_m4a_dec_seek.a
#ifdef CONFIG_NEW_M4A_DEC_ENABLE
#define CONFIG_AAC_DEC_ENABLE
#endif
#define CONFIG_VIRTUAL_DEV_ENC_ENABLE
#define CONFIG_SPEEX_ENC_ENABLE
#define CONFIG_OPUS_ENC_ENABLE
//#define CONFIG_VAD_ENC_ENABLE
#endif
#if __FLASH_SIZE__ > (2 * 1024 * 1024)
#define CONFIG_DTS_DEC_ENABLE
#define CONFIG_ADPCM_DEC_ENABLE
#define CONFIG_MP3_ENC_ENABLE
#define CONFIG_WMA_DEC_ENABLE
#define CONFIG_AMR_DEC_ENABLE
#define CONFIG_APE_DEC_ENABLE
#define CONFIG_FLAC_DEC_ENABLE
#define CONFIG_SPEEX_DEC_ENABLE
#define CONFIG_ADPCM_ENC_ENABLE
#define CONFIG_OPUS_DEC_ENABLE
#define CONFIG_AMR_ENC_ENABLE
#define CONFIG_AEC_ENC_ENABLE
#define CONFIG_DNS_ENC_ENABLE
#define CONFIG_OGG_DEC_ENABLE

//#define CONFIG_VIRTUAL_BASS_ENABLE             //虚拟低音
// #define CONFIG_SPECTRUM_FFT_EFFECT_ENABLE    //频谱运算
// #define CONFIG_REVERB_MODE_ENABLE            //打开混响功能
//#define CONFIG_AUDIO_MIX_ENABLE              //打开叠音功能
//#define CONFIG_AUDIO_PS_ENABLE               //打开变调变速功能
#endif

#ifdef CONFIG_AEC_ENC_ENABLE
//#define CONFIG_USB_AUDIO_AEC_ENABLE          //usb mic使能回声消除功能
#define CONFIG_AEC_LINEIN_CHANNEL_ENABLE     //AEC回采使用硬件通道数据
//#define CONFIG_AEC_USE_PLAY_MUSIC_ENABLE       //播歌时需要使用AEC
//#define CONFIG_AEC_USE_SMS_TDE   //使用tde算法, 编码参数req.enc.aec_enable设置为2
#endif

#define CONFIG_ALL_ADC_CHANNEL_OPEN_ENABLE   //四路ADC硬件全开

#ifndef CONFIG_AUDIO_LINEIN_SAMPLERATE
#define CONFIG_AUDIO_LINEIN_SAMPLERATE 48000 //LINEIN采样率
#endif
#ifndef CONFIG_AUDIO_LINEIN_CHANNEL
#define CONFIG_AUDIO_LINEIN_CHANNEL    1     //LIENIN通道数
#endif
#ifndef CONFIG_AUDIO_LINEIN_CHANNEL_MAP
#define CONFIG_AUDIO_LINEIN_CHANNEL_MAP (BIT(3)) //LIENIN通道选择
#endif
#ifndef CONFIG_AUDIO_LINEIN_ADC_GAIN
#define CONFIG_AUDIO_LINEIN_ADC_GAIN   60    //LIENIN的模拟增益
#endif

#endif


//*********************************************************************************//
//                                  路径配置                                       //
//*********************************************************************************//
#if TCFG_SD0_ENABLE
#define CONFIG_STORAGE_PATH 	"storage/sd0"  //定义对应SD0的路径
#define SDX_DEV					"sd0"
#endif

#if TCFG_SD1_ENABLE && !TCFG_SD0_ENABLE
#define CONFIG_STORAGE_PATH 	"storage/sd1" //定义对应SD1的路径
#define SDX_DEV					"sd1"
#endif

#ifndef CONFIG_STORAGE_PATH
#define CONFIG_STORAGE_PATH		"storage/sdx" //不使用SD定义对应别的路径，防止编译出错
#define SDX_DEV					"sdx"
#endif

#define CONFIG_UDISK_STORAGE_PATH	"storage/udisk0"

#define CONFIG_ROOT_PATH            CONFIG_STORAGE_PATH"/C/" //定义对应SD文件系统的根目录路径
#define CONFIG_UDISK_ROOT_PATH     	CONFIG_UDISK_STORAGE_PATH"/C/" //定义对应U盘文件系统的根目录路径

#define CONFIG_MUSIC_PATH_SD        CONFIG_ROOT_PATH
#define CONFIG_MUSIC_PATH_SD0       "storage/sd0/C/"
#define CONFIG_MUSIC_PATH_SD1       "storage/sd1/C/"
#define CONFIG_MUSIC_PATH_UDISK     CONFIG_UDISK_ROOT_PATH
#define CONFIG_MUSIC_PATH_UDISK0    "storage/udisk0/C/"
#define CONFIG_MUSIC_PATH_UDISK1    "storage/udisk1/C/"

#define CONFIG_EQ_FILE_NAME                 "mnt/sdfile/res/cfg/eq_cfg_hw.bin"
#if defined CONFIG_VOICE_PROMPT_FILE_SAVE_IN_RESERVED_EXPAND_ZONE
#define CONFIG_MUSIC_PATH_FLASH             "mnt/sdfile/EXT_RESERVED/EXT_RESERVED/aupackres/"
#elif defined CONFIG_VOICE_PROMPT_FILE_SAVE_IN_RESERVED_ZONE
#define CONFIG_MUSIC_PATH_FLASH             "mnt/sdfile/app/app_area_head/aupackres/"
#else
#define CONFIG_MUSIC_PATH_FLASH             "mnt/sdfile/res/"
#endif
#ifdef CONFIG_AUDIO_ENABLE
#if defined CONFIG_VOICE_PROMPT_FILE_SAVE_IN_RESERVED_EXPAND_ZONE
#define CONFIG_VOICE_PROMPT_FILE_PATH       "mnt/sdfile/EXT_RESERVED/aupackres/tone/"
#elif defined CONFIG_VOICE_PROMPT_FILE_SAVE_IN_RESERVED_ZONE
#define CONFIG_VOICE_PROMPT_FILE_PATH       "mnt/sdfile/app/aupackres/tone/"
#else
#define CONFIG_VOICE_PROMPT_FILE_PATH       "mnt/sdfile/res/audlogo/"
#endif
#endif

#define CONFIG_DOUBLE_BANK_ENABLE           0//1: 双备份方式升级 0:单备份方式升级

#define CONFIG_UPGRADE_FILE_NAME            "update.ufw"
#define CONFIG_UPGRADE_PATH                 CONFIG_ROOT_PATH\
											CONFIG_UPGRADE_FILE_NAME	//备份方式升级

#define CONFIG_SD0_UPGRADE_PATH             "storage/sd0/C/"CONFIG_UPGRADE_FILE_NAME	//备份方式升级
#define CONFIG_SD1_UPGRADE_PATH             "storage/sd1/C/"CONFIG_UPGRADE_FILE_NAME	//备份方式升级
#define CONFIG_UDISK0_UPGRADE_PATH          "storage/udisk0/C/"CONFIG_UPGRADE_FILE_NAME	//备份方式升级
#define CONFIG_UDISK1_UPGRADE_PATH          "storage/udisk1/C/"CONFIG_UPGRADE_FILE_NAME	//备份方式升级


//*********************************************************************************//
//                                  EQ配置                                         //
//*********************************************************************************//
#define CONFIG_VOLUME_TAB_TEST_ENABLE       0     //音量表测试
//EQ配置，使用在线EQ时，EQ文件和EQ模式无效。有EQ文件时，默认不用EQ模式切换功能
#if defined CONFIG_AUDIO_ENABLE && __FLASH_SIZE__ > (1 * 1024 * 1024)
#define TCFG_EQ_ENABLE                      1     //支持EQ功能
#else
#define TCFG_EQ_ENABLE                      0     //关闭EQ功能
#endif
#define TCFG_EQ_ONLINE_ENABLE               0     //支持在线EQ调试
#define TCFG_HW_SOFT_EQ_ENABLE              1     //前3段使用软件运算
#define TCFG_LIMITER_ENABLE                 1     //限幅器
#define TCFG_DRC_ENABLE                     TCFG_LIMITER_ENABLE
#define TCFG_EQ_FILE_ENABLE                 1     //从bin文件读取eq配置数据
#define TCFG_EQ_MODE_CHOOSE                 0     //0:多通道共用1个eq, drc功能丰富; 1:多通道独立eq,独立drc, 请替换对应的bin文件

#define TCFG_AEC_UL_EQ_ENABLE               1     //支持编码eq功能

#if (TCFG_EQ_MODE_CHOOSE == 0)
#define TCFG_AUDIO_MDRC_ENABLE              2     //0:不使能低中DRC 1: 多带分频器使能 2: 多带分频后，再做一次全带处理使能
#define TCFG_LAST_WHOLE_DRC_ENABLE          1     //0:不使能最后的全带DRC; 1:使能
#elif (TCFG_EQ_MODE_CHOOSE == 1)
#define TCFG_EQ_DIVIDE_ENABLE               1     //0:前后通道共用EQ/DRC  1:支持EQ/DRC前后声道独立处理
#define TCFG_EQ_SPILT_ENABLE                1     //0:左右通道共用EQ  1:支持EQ左右声道独立处理
#define TCFG_DRC_SPILT_ENABLE               1     //0:左右通道共用DRC 1:支持DRC左右声道独立处理
#endif
//EQ在线调试通信类型
#define TCFG_NULL_COMM                      0     //不支持通信
#define TCFG_USB_COMM                       1     //USB通信
#if TCFG_EQ_ONLINE_ENABLE && defined EQ_CORE_V1
#define TCFG_COMM_TYPE                      TCFG_USB_COMM
#else
#define TCFG_COMM_TYPE                      TCFG_NULL_COMM
#endif


//*********************************************************************************//
//                                  VIDEO配置                                      //
//*********************************************************************************//
#ifdef CONFIG_VIDEO_ENABLE

#ifdef CONFIG_SCAN_PEN_ENABLE
#define CONFIG_VIDEO1_ENABLE
#define CONFIG_SPI_VIDEO_ENABLE
#define CONFIG_VIDEO_720P
#endif

//#define CONFIG_MASS_PRODUCTION_ENABLE //启用产测模式
//#define CONFIG_OSD_ENABLE			/* 视频OSD时间戳开关 */
#define CONFIG_VIDEO_REC_PPBUF_MODE	/*视频使用乒乓BUF模式(图传<=20帧),关闭则用lbuf模式(图传>20帧和写卡录像),缓冲区大小配置video_buf_config.h*/
//#define CONFIG_VIDEO_SPEC_DOUBLE_REC_MODE	/* 视频支持双路莫模式(一路实时流、一路录SD卡)*/

#ifdef CONFIG_MASS_PRODUCTION_ENABLE
#define STA_WIFI_SSID     "test"    //量产模式的路由器名称
#define STA_WIFI_PWD      "12345678"  //量产模式的路由器密码
//#define CONFIG_PRODUCTION_IO_PORT			IO_PORTB_01 //配置进入量产莫模式的IO
//#define CONFIG_PRODUCTION_IO_STATE		0 			//配置进入量产莫模式的IO状态：0低电平，1高电平
#endif

//*********************************************************************************//
//                             编码图片分辨率                                      //
//*********************************************************************************//
//#define CONFIG_VIDEO_720P
#ifdef CONFIG_VIDEO_720P
#define CONFIG_VIDEO_IMAGE_W    1280
#define CONFIG_VIDEO_IMAGE_H    720
#else
#define CONFIG_VIDEO_IMAGE_W    640
#define CONFIG_VIDEO_IMAGE_H    480
#undef  CONFIG_WMA_DEC_ENABLE
#endif

//*********************************************************************************//
//                             视频流相关配置                                      //
//*********************************************************************************//
#define VIDEO_REC_AUDIO_SAMPLE_RATE		0    //视频流的音频采样率,注意：硬件没MIC则为0
#define VIDEO_REC_FPS 					20   //录像SD卡视频帧率设置,0为默认

#define CONFIG_USR_VIDEO_ENABLE		//用户VIDEO使能
#define CONFIG_USR_PATH 	"192.168.1.1:8000" //用户路径，可随意设置，video_rt_usr.c的init函数看进行读取

#endif

//*********************************************************************************//
//                                  USB配置                                        //
//*********************************************************************************//
#ifndef TCFG_PC_ENABLE
#if TCFG_EQ_ONLINE_ENABLE && defined EQ_CORE_V1
#define TCFG_PC_ENABLE                      1     //使用USB从机功能一定要打开
#else
#define TCFG_PC_ENABLE                      0     //使用USB从机功能一定要打开
#endif
#endif
#define USB_PC_NO_APP_MODE                  2
#define USB_MALLOC_ENABLE                   1
#ifndef USB_DEVICE_CLASS_CONFIG
#if TCFG_EQ_ONLINE_ENABLE && defined EQ_CORE_V1
#define USB_DEVICE_CLASS_CONFIG             (CDC_CLASS)
#else
#define USB_DEVICE_CLASS_CONFIG             (MASSSTORAGE_CLASS)
#endif
#endif
#ifndef TCFG_UDISK_ENABLE
#define TCFG_UDISK_ENABLE                   0     //U盘主机功能
#endif
#ifndef TCFG_HOST_AUDIO_ENABLE
#define TCFG_HOST_AUDIO_ENABLE              0     //uac主机功能，用户需要自己补充uac_host_demo.c里面的两个函数
#endif
#ifndef TCFG_HOST_UVC_ENABLE
#define TCFG_HOST_UVC_ENABLE                0     //打开USB 后拉摄像头功能，需要使能住机模式
#endif
#ifdef CONFIG_LTE_PHY_ENABLE
#define TCFG_HOST_WIRELESS_ENABLE           1
#define TCFG_ADB_ENABLE                     1     //usb虚拟网卡
#endif
#ifndef TCFG_HOST_CDC_ENABLE
#define TCFG_HOST_CDC_ENABLE                0     //打开USB CDC主机
#endif

#include "usb_std_class_def.h"
#include "usb_common_def.h"

#ifndef TCFG_VIR_UDISK_ENABLE
#define TCFG_VIR_UDISK_ENABLE               0     //虚拟U盘
#endif
#define TCFG_USER_VIRTUAL_PLAY_ENABLE       TCFG_VIR_UDISK_ENABLE
#define TCFG_VIR_UPDATE_ENABLE              0     //虚拟U盘升级,依赖（TCFG_PC_ENABLE = 1 && TCFG_VIR_UDISK_ENABLE = 1）
#define TCFG_USER_VIRTUAL_PLAY_SAMPLERATE   44100


//*********************************************************************************//
//                  EXTFLASH配置(截取flash中的一段空间作为extflash)                //
//*********************************************************************************//
// #define TCFG_EXTFLASH_ENABLE
#ifdef TCFG_EXTFLASH_ENABLE
// #define TCFG_EXTFLASH_UDISK_ENABLE     //将extflash枚举为udisk设备
#endif


//*********************************************************************************//
//                                    FM配置                                       //
//*********************************************************************************//
#define CONFIG_FM_DEV_ENABLE                0        //打开外挂FM模块功能
#define CONFIG_FM_LINEIN_ADC_GAIN           100
#define CONFIG_FM_LINEIN_ADC_CHANNEL        3        //FM音频流回采AD通道
#define TCFG_FM_QN8035_ENABLE               1
#define TCFG_FM_BK1080_ENABLE               0
#define TCFG_FM_RDA5807_ENABLE              0


//*********************************************************************************//
//                                    电源配置                                     //
//*********************************************************************************//
// #define CONFIG_LOW_POWER_ENABLE              		//低功耗开关
#define TCFG_LOWPOWER_BTOSC_DISABLE			0
#ifdef CONFIG_LOW_POWER_ENABLE
#define TCFG_LOWPOWER_LOWPOWER_SEL			(RF_SLEEP_EN | SYS_SLEEP_EN | RF_FORCE_SYS_SLEEP_EN | SYS_SLEEP_BY_IDLE)
#else
#define TCFG_LOWPOWER_LOWPOWER_SEL			0
#endif

#ifdef CONFIG_INTERNAL_VDDIO_POWER_SUPPLY_ENABLE
#ifdef CONFIG_NO_SDRAM_ENABLE
#define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_33V       //强VDDIO电压档位
#else
#define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_35V       //强VDDIO电压档位，采用内部VDDIO供电，带sdram的封装实际工作电压至少要满足3.3V
#endif
#else
#define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_32V       //强VDDIO电压档位，不要高于外部DCDC的电压
#endif

#ifdef CONFIG_RTC_ENABLE
#define TCFG_LOWPOWER_VDDIOW_LEVEL			VDDIOW_VOL_32V       //弱VDDIO电压档位。RTCVDD电压低于3.2V可能不走时，因此RTCVDD由IOVDD供电时，VDDIOW应设置为VDDIOW_VOL_32V档
#else
#define TCFG_LOWPOWER_VDDIOW_LEVEL			VDDIOW_VOL_21V       //弱VDDIO电压档位
#endif

#define VDC14_VOL_SEL_LEVEL			        VDC14_VOL_SEL_140V   //RF1.4V电压档位
#define SYSVDD_VOL_SEL_LEVEL				SYSVDD_VOL_SEL_126V  //内核电压档位值

//*********************************************************************************//
////                                  AI配置(RCSP)                                       //
////*********************************************************************************//

#define CONFIG_APP_BT_ENABLE                0 // AI(RCSP)功能、流程总开关

///注意：以下功能配置只选其一
#if CONFIG_APP_BT_ENABLE
#define	RCSP_MODE			   	            3 //RCSP_MODE_WATCH
#define TCFG_DEV_MANAGER_ENABLE             1
#define TCFG_SD_ALWAY_ONLINE_ENABLE         1
#define	ANCS_CLIENT_EN			 			0
#define	BLE_CLIENT_EN 			 			0
#define	AI_APP_PROTOCOL 			 		0
#define TUYA_DEMO_EN              			0
#else
#define RCSP_MODE				            0 //RCSP_MODE_OFF
#define	ANCS_CLIENT_EN			 			0
#define	BLE_CLIENT_EN 			 			0
#define AI_APP_PROTOCOL           			0
#define TUYA_DEMO_EN              			0
#endif


///注意：对应的第三方应用平台是否需要支持语音上报， 如果需要请使能BT_MIC_EN
//对应的语音编码参数在mic_rec.c中定义
//#define BT_MIC_EN                  	 1
//

//*********************************************************************************//
//                                  BT_BLE配置                                     //
//*********************************************************************************//
#ifdef CONFIG_BT_ENABLE

#define BT_EMITTER_EN     1
#define BT_RECEIVER_EN    2

#define CONFIG_POWER_ON_ENABLE_EMITTER            0     //开机自动打开发射器
#define CONFIG_POWER_ON_ENABLE_BT                 0     //开机自动打开经典蓝牙
#define CONFIG_POWER_ON_ENABLE_BLE                0     //开机自动打开BLE
#define TCFG_BD_NUM                               1     //可连接设备数量，可支持同时连接2台设备
#define TCFG_USER_BT_CLASSIC_ENABLE               1     //经典蓝牙功能
#define TCFG_USER_TWS_ENABLE                      0     //tws功能使能
#define TCFG_USER_BLE_ENABLE                      1     //BLE功能使能
#define TCFG_USER_EDR_ENABLE                      0     //EDR用户自定义协议功能
#if __FLASH_SIZE__ > (1 * 1024 * 1024)
#define TCFG_USER_EMITTER_ENABLE                  1     //蓝牙发射功能
#else
#define TCFG_USER_EMITTER_ENABLE                  0     //蓝牙发射功能
#endif
#define TCFG_BT_SNIFF_ENABLE                      1
#ifdef CONFIG_NEW_M4A_DEC_ENABLE
#define TCFG_BT_SUPPORT_AAC                       1     //蓝牙AAC格式支持
#else
#define TCFG_BT_SUPPORT_AAC                       0     //蓝牙AAC格式支持
#endif
#define BT_SUPPORT_MUSIC_VOL_SYNC                 1     //音量同步
#define BT_SUPPORT_DISPLAY_BAT                    1     //电池电量同步显示功能
#define BT_SUPPORT_EMITTER_AUTO_A2DP_START        1     //发射器连上后自动打开A2DP流
#define BT_SUPPORT_RECEIVER_AUTO_A2DP_START       1     //接收器器连上后自动打开A2DP流
#define BT_SUPPORT_EMITTER_PAGE_SCAN              0     //发射器打开可发现性
#define BT_SUPPORT_WIFI_CFG_COEXISTENCE           0     //wifi配网时保持蓝牙连接

#if TCFG_USER_EDR_ENABLE
#define SPP_TRANS_DATA_EN                         1
#endif

#if defined CONFIG_CPU_WL82 && defined CONFIG_ASR_ALGORITHM && defined CONFIG_VIDEO_ENABLE
#define CONFIG_BT_RX_BUFF_SIZE  (8 * 1024 - 512)
#define CONFIG_BT_TX_BUFF_SIZE  (6 * 1024 - 512)
#else
#if defined CONFIG_VIDEO_720P || TCFG_PC_ENABLE || TCFG_BD_NUM > 1 || (!defined CONFIG_VIDEO_ENABLE && !defined CONFIG_NO_SDRAM_ENABLE)
#define CONFIG_BT_RX_BUFF_SIZE  (17 * 1024 + 400)
#define CONFIG_BT_TX_BUFF_SIZE  (10 * 1024 + 512)
#else
#define CONFIG_BT_RX_BUFF_SIZE  (11 * 1024)
#define CONFIG_BT_TX_BUFF_SIZE  (6 * 1024)
#endif
#endif

#if TCFG_USER_BLE_ENABLE

#define TCFG_BLE_SECURITY_EN                      0     //配对加密使能

#ifdef CONFIG_NET_ENABLE

#define CONFIG_BLE_MESH_ENABLE                    0

#ifdef CONFIG_DUI_SDK_ENABLE
#define BT_NET_CFG_DUI_EN                         1     //从机 思必驰配网专用
#else
#define BT_NET_CFG_DUI_EN                         0     //从机 思必驰配网专用
#endif

#ifdef CONFIG_TURING_SDK_ENABLE
#define BT_NET_CFG_TURING_EN                      1     //从机 图灵配网专用
#else
#define BT_NET_CFG_TURING_EN                      0     //从机 图灵配网专用
#endif

#ifdef CONFIG_GIZWITS_SDK_ENABLE
#define BT_NET_CFG_GIZWITS_EN                     1     //从机 腾讯云配网专用
#else
#define BT_NET_CFG_GIZWITS_EN                     0     //从机 腾讯云配网专用
#endif

#ifdef CONFIG_TVS_SDK_ENABLE
#define BT_NET_CFG_TENCENT_EN                     1     //从机 腾讯云配网专用
#else
#define BT_NET_CFG_TENCENT_EN                     0     //从机 腾讯云配网专用
#endif

#ifdef CONFIG_SXY_QYAI_ENABLE
#define BT_NET_CFG_QYAI_EN                        1    //从机 数芯语-轻语AI-配网专用
#undef CONFIG_POWER_ON_ENABLE_BLE
#define CONFIG_POWER_ON_ENABLE_BLE                0    //关闭自动打开BLE
#else
#define BT_NET_CFG_QYAI_EN                        0    //从机 数芯语-轻语AI-配网专用
#endif

#if BT_NET_CFG_TURING_EN + BT_NET_CFG_DUI_EN + BT_NET_CFG_TENCENT_EN + CONFIG_APP_BT_ENABLE + BT_NET_CFG_GIZWITS_EN > 0
#define BT_NET_CFG_EN                             1     //从机 杰理配网专用
#else
#define BT_NET_CFG_EN                             1     //从机 杰理配网专用
#endif

#define TRANS_DATA_EN                             0     //从机 传输数据

#else

#define BT_NET_CFG_DUI_EN                         0     //从机 思必驰配网专用
#define BT_NET_CFG_TURING_EN                      0     //从机 图灵配网专用
#define BT_NET_CFG_TENCENT_EN                     0     //从机 腾讯云配网专用
#define BT_NET_CFG_EN                             0     //从机 配网专用
#define TRANS_DATA_EN                             1     //从机 传输数据

#endif

#define XIAOMI_EN                                 0     //从机 mi_server
#define BT_NET_CENTRAL_EN                         0     //主机 client角色
#define BT_NET_HID_EN                             0     //从机 hid
#define TRANS_MULTI_BLE_EN                        0     //多机通讯
#define APP_NONCONN_24G				  			  0		//2.4g加密通讯

#if (TRANS_MULTI_BLE_EN + BT_NET_CFG_TURING_EN + BT_NET_CFG_DUI_EN + BT_NET_CFG_EN + BT_NET_HID_EN + TRANS_DATA_EN + XIAOMI_EN + CONFIG_APP_BT_ENABLE + APP_NONCONN_24G > 1)
#error "they can not enable at the same time,just select one!!!"
#endif
#endif

#if TRANS_MULTI_BLE_EN
#define TRANS_MULTI_BLE_SLAVE_NUMS                1
#define TRANS_MULTI_BLE_MASTER_NUMS               1
#endif

#if (CONFIG_BLE_MESH_ENABLE || TCFG_USER_EMITTER_ENABLE) && defined CONFIG_WIFI_ENABLE && !defined CONFIG_LOW_POWER_ENABLE
#define CONFIG_WIFI_IDLE_RESUME_BASEBAND_ENABLE
#endif

#endif


//*********************************************************************************//
//                                  UI配置                                         //
//*********************************************************************************//
#ifdef CONFIG_UI_ENABLE

// #define CONFIG_NES_DEMO //NES模拟器

#define CONFIG_VIDEO_DEC_ENABLE             1  //打开视频解码器
#define TCFG_LCD_ENABLE                     1  //使用lcd屏幕
#define TCFG_USE_SD_ADD_UI_FILE             0  //使用SD卡加载资源文件

#if TCFG_LCD_ENABLE
#define TCFG_LCD_ILI9481_ENABLE             1
#define TCFG_LCD_ILI9341_ENABLE             0
#define TCFG_LCD_ST7789S_ENABLE             0
#define TCFG_LCD_ST7789V_ENABLE             0
#define TCFG_LCD_ST7735S_ENABLE             0
#define TCFG_LCD_480x272_8BITS              0
#endif

#if TCFG_LCD_ILI9341_ENABLE
#define TCFG_TOUCH_GT911_ENABLE             1
#else
#define TCFG_TOUCH_GT911_ENABLE             0
#endif

#if TCFG_LCD_ILI9481_ENABLE
#define TCFG_TOUCH_FT6236_ENABLE            1
#else
#define TCFG_TOUCH_FT6236_ENABLE            0
#endif

#if TCFG_LCD_480x272_8BITS || TCFG_LCD_ST7789V_ENABLE || TCFG_LCD_ILI9341_ENABLE || TCFG_LCD_ILI9481_ENABLE
#define HORIZONTAL_SCREEN                   0//0为使用竖屏
#else
#define HORIZONTAL_SCREEN                   1//1为使能横屏配置
#endif

#if TCFG_LCD_ST7789S_ENABLE || TCFG_LCD_ILI9341_ENABLE || TCFG_LCD_ILI9481_ENABLE
#define USE_LCD_TE                          1
#endif

#endif//CONFIG_UI_ENABLE


#if !defined CONFIG_DEBUG_ENABLE || defined CONFIG_LIB_DEBUG_DISABLE
#define LIB_DEBUG    0
#else
#define LIB_DEBUG    1
#endif
#define CONFIG_DEBUG_LIB(x)         (x & LIB_DEBUG)

#include "video_buf_config.h"

//*********************************************************************************//
//                                  C++ 支持配置                                   //
//*********************************************************************************//
#define CONFIG_CXX_SUPPORT    // 使能C++环境支持

#endif

