#ifndef __RCSP_DEFINE_H__
#define __RCSP_DEFINE_H__

#define RCSP_MODE_OFF	                (0)			// RCSP功能关闭
#define RCSP_MODE_COMMON                (1)			// 适用于通用设备，如hid
#define RCSP_MODE_SOUNDBOX              (2)			// 适用于音箱SDK
#define RCSP_MODE_WATCH		            (3)			// 适用于手表SDK
#define RCSP_MODE_EARPHONE              (4)			// 适用于耳机SDK
#define RCSP_MODE_EARPHONE_V2022        (5)			// 适用于耳机SDKV2022架构

#define		RCSP_SDK_TYPE_AC690X			0x0
#define		RCSP_SDK_TYPE_AC692X			0x1
#define 	RCSP_SDK_TYPE_AC693X			0x2
#define 	RCSP_SDK_TYPE_AC695X 		0x3
#define		RCSP_SDK_TYPE_AC697X 		0x4
#define		RCSP_SDK_TYPE_AC696X 		0x5
#define		RCSP_SDK_TYPE_AC696X_TWS		0x6
#define		RCSP_SDK_TYPE_AC695X_WATCH	0x8
#define		RCSP_SDK_TYPE_AC701N_WATCH	0x9



#endif
