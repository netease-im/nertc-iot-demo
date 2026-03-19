
# make 编译并下载
# make VERBOSE=1 显示编译详细过程
# make clean 清除编译临时文件
#
# 注意： Linux 下编译方式：
#     1. 从 http://pkgman.jieliapp.com/doc/all 处找到下载链接
#     2. 下载后，解压到 /opt/jieli 目录下，保证
#       /opt/jieli/common/bin/clang 存在（注意目录层次）
#     3. 确认 ulimit -n 的结果足够大（建议大于8096），否则链接可能会因为打开文件太多而失败
#       可以通过 ulimit -n 8096 来设置一个较大的值
#

# 工具路径设置
ifeq ($(OS), Windows_NT)
# Windows 下工具链位置
TOOL_DIR := C:/JL/pi32/bin
CC    := clang.exe
CXX   := clang.exe
LD    := pi32v2-lto-wrapper.exe
AR    := llvm-ar.exe
MKDIR := mkdir_win -p
RM    := rm -rf

SYS_LIB_DIR := 
SYS_INC_DIR := 
EXT_CFLAGS  := # Windows 下不需要 -D__SHELL__
export PATH:=$(TOOL_DIR);$(PATH)

## 后处理脚本
FIXBAT          := ../../../../tools/utils/fixbat.exe # 用于处理 utf8->gbk 编码问题
POST_SCRIPT     := ../../../../cpu/wl82/tools/download.bat
RUN_POST_SCRIPT := ..\..\..\..\cpu\wl82\tools\download.bat
else
# Linux 下工具链位置
TOOL_DIR := /opt/jieli/pi32v2/bin
CC    := clang
CXX   := clang
LD    := lto-wrapper
AR    := lto-ar
MKDIR := mkdir -p
RM    := rm -rf
export OBJDUMP := $(TOOL_DIR)/objdump
export OBJCOPY := $(TOOL_DIR)/objcopy
export OBJSIZEDUMP := $(TOOL_DIR)/objsizedump

SYS_LIB_DIR := $(TOOL_DIR)/../lib/r3
SYS_INC_DIR := $(TOOL_DIR)/../include
EXT_CFLAGS  := -D__SHELL__ # Linux 下需要这个保证正确处理 download.c
export PATH:=$(TOOL_DIR):$(PATH)

## 后处理脚本
FIXBAT          := touch # Linux下不需要处理 bat 编码问题
POST_SCRIPT     := ../../../../cpu/wl82/tools/download.sh
RUN_POST_SCRIPT := bash $(POST_SCRIPT)
endif

CC  := $(TOOL_DIR)/$(CC)
CXX := $(TOOL_DIR)/$(CXX)
LD  := $(TOOL_DIR)/$(LD)
AR  := $(TOOL_DIR)/$(AR)
# 输出文件设置
OUT_ELF   := ../../../../cpu/wl82/tools/sdk.elf
OBJ_FILE  := $(OUT_ELF).objs.txt
# 编译路径设置
BUILD_DIR := objs
# 工程路径前缀
ROOT_PREFIX := ../../../..

# 编译参数设置
CFLAGS := \
	-target pi32v2 \
	-integrated-as \
	-mcpu=r3 \
	-DUSE_NERTC_AI_SERVER \
	-mfprev1 \
	-Wuninitialized \
	-Wno-invalid-noreturn \
	-fno-common \
	-integrated-as \
	-Oz \
	-g \
	-flto \
	-fallow-pointer-null \
	-fprefer-gnu-section \
	-femulated-tls \
	-Wno-shift-negative-value \
	-Wframe-larger-than=2560 \
	-mllvm -pi32v2-large-program=true \
	-fms-extensions \
	-w \
	-fno-unwind-tables \
	-ffunction-sections \
	-fdata-sections \
	-fmessage-length=0 \


# C++额外的编译参数
CXXFLAGS := \
	-std=gnu++14 \
	-fno-exceptions \
	-fno-rtti \


# 宏定义
DEFINES := \
	-DSUPPORT_MS_EXTENSIONS \
	-DCONFIG_RELEASE_ENABLE \
	-DCONFIG_CPU_WL82 \
	-DCONFIG_FREE_RTOS_ENABLE \
	-DCONFIG_THREAD_ENABLE \
	-DCONFIG_AUDIO_ONCHIP \
	-D__GCC_PI32V2__ \
	-DCONFIG_NEW_ECC_ENABLE \
	-DCONFIG_EQ_SUPPORT_ASYNC \
	-DEQ_CORE_V1 \
	-DCONFIG_VIDEO_DEC_EN \
	-DCONFIG_VIDEO_OSD_EN \
	-DCONFIG_SFC_ENABLE \
	-DCONFIG_AUDIO_ENABLE \
	-DCONFIG_JLFAT_ENABLE \
	-DCONFIG_RTOS_AND_MM_LIB_CODE_SECTION_IN_SDRAM \
	-DCONFIG_NET_ENABLE=1 \
	-DCONFIG_BT_ENABLE=1 \
	-DEVENT_HANDLER_NUM_CONFIG=2 \
	-DEVENT_TOUCH_ENABLE_CONFIG=0 \
	-DEVENT_POOL_SIZE_CONFIG=256 \
	-DCONFIG_EVENT_KEY_MAP_ENABLE=0 \
	-DTIMER_POOL_NUM_CONFIG=15 \
	-DAPP_ASYNC_POOL_NUM_CONFIG=0 \
	-DVFS_ENABLE=1 \
	-DUSE_SDFILE_NEW=1 \
	-DSDFILE_STORAGE=1 \
	-DVFS_FILE_POOL_NUM_CONFIG=8 \
	-DVM_MAX_PAGE_ALIGN_SIZE_CONFIG=64*1024 \
	-DVM_MAX_SECTOR_ALIGN_SIZE_CONFIG=64*1024 \
	-DVM_ITEM_MAX_NUM=256 \
	-DCONFIG_TWS_ENABLE \
	-DCONFIG_BTCTRLER_TASK_DEL_ENABLE \
	-DCONFIG_LMP_CONN_SUSPEND_ENABLE \
	-DCONFIG_LMP_REFRESH_ENCRYPTION_KEY_ENABLE \
	-DCONFIG_ITEM_FORMAT_VM \
	-DJIELI_RTOS \
	-DJIELI_791N_RTOS \
	-DRTC_JIELI_RTOS \
	-D_XOPEN_SOURCE=700 \
	-D_GNU_SOURCE \
	-D__ELF__ \


DEFINES += $(EXT_CFLAGS) # 额外的一些定义

# 头文件搜索路径
INCLUDES := \
	-I../../../../include_lib/c++ \
	-I../../../../include_lib/c++/include \
	-I../../../../include_lib/newlib/include \
	-I../../../../include_lib/c++/simple_pthread \
	-I../../../../include_lib \
	-I../../../../include_lib/driver \
	-I../../../../include_lib/driver/device \
	-I../../../../include_lib/driver/cpu/wl82 \
	-I../../../../include_lib/system \
	-I../../../../include_lib/system/generic \
	-I../../../../include_lib/btctrler \
	-I../../../../include_lib/btctrler/port/wl82 \
	-I../../../../include_lib/update \
	-I../../../../include_lib/btstack/third_party/common \
	-I../../../../include_lib/btstack/third_party/rcsp \
	-I../../../../include_lib/utils \
	-I../../../../include_lib/utils/syscfg \
	-I../../../../include_lib/utils/event \
	-I../../../../include_lib/media \
	-I../../../../include_lib/media/cpu/wl82 \
	-I../../../../include_lib/btstack \
	-I../../../../apps \
	-I../../../../apps/wifi_story_machine \
	-I../../../../apps/wifi_story_machine/include \
	-I../../../../apps/wifi_story_machine/board/wl82 \
	-I../../../../apps/common \
	-I../../../../apps/common/include \
	-I../../../../apps/common/ui/include \
	-I../../../../apps/common/config/include \
	-I../../../../apps/common/system \
	-I../../../../include_lib/utils/ui \
	-I../../../../include_lib/driver/device/video \
	-I../../../../include_lib/net \
	-I../../../../include_lib/net/mbedtls_3_4_0 \
	-I../../../../include_lib/net/server \
	-I../../../../include_lib/net/wifi_module \
	-I../../../../include_lib/utils/img_stitch \
	-I../../../../include_lib/net/hostapdandwpa_supplicant \
	-I../../../../include_lib/net/lwip_2_2_0/lwip/src/include \
	-I../../../../include_lib/net/lwip_2_2_0/lwip/src/include/lwip \
	-I../../../../include_lib/net/lwip_2_2_0/lwip/port \
	-I../../../../include_lib/net/lwip_2_2_0/lwip/app \
	-I../../../../include_lib/net/lwip_2_2_0/lwip/app/ctp \
	-I../../../../include_lib/net/lwip_2_2_0/lwip/app/cdp \
	-I../../../../apps/common/example/third_party/littlefs \
	-I../../../../apps/common/rf_fcc_tool/include \
	-I../../../../apps/common/usb \
	-I../../../../apps/common/usb/device \
	-I../../../../apps/common/usb/host \
	-I../../../../apps/common/usb/include/host \
	-I../../../../apps/common/usb/include \
	-I../../../../include_lib/driver/device/usb \
	-I../../../../include_lib/driver/device/usb/device \
	-I../../../../include_lib/driver/device/usb/host \
	-I../../../../apps/common/net \
	-I../../../../apps/common/net/testbox/include \
	-I../../../../apps/common/ble/include \
	-I../../../../include_lib/utils/btmesh \
	-I../../../../include_lib/utils/btmesh/adaptation \
	-I../../../../apps/common/LLM/audio \
	-I../../../../apps/common/LLM/volc/inc \
	-I../../../../apps/common/LLM/volc \
	-I../../../../apps/common/LLM/extra \
	-I../../../../include_lib/net/cJSON_common \
	-I../../../../include_lib/net/curl/include \
	-I../../../../include_lib/net/lwip_2_2_0/lwip/src/include/compat/posix \
	-I../../../../include_lib/net/lwip_2_2_0/lwip/src/include/compat/stdc \
	-I../../../../include_lib/system/os \
	-I../../../../include_lib/net/libwebsockets/include \
	-I../../../../include_lib/net/libwebsockets/include/libwebsockets \
	-I../../../../apps/common/LLM/onesdk/src \
	-I../../../../apps/common/LLM/onesdk/src/aigw \
	-I../../../../apps/common/LLM/onesdk/src/util \
	-I../../../../apps/common/LLM/onesdk/src/aws \
	-I../../../../apps/common/LLM/onesdk/include \
	-I../../../../apps/common/LLM/onesdk/include/iot \
	-I../../../../apps/common/LLM/onesdk/include/protocols \
	-I../../../../apps/common/LLM/onesdk/include/thing_model \
	-I../../../../apps/common/LLM/onesdk/include/platform/plat \
	-I../../../../apps/common/LLM/onesdk/platform \
	-I../../../../apps/common/LLM/onesdk \
	-I../../../../include_lib/net/web_socket \
	-I../../../../apps/common/LLM/ifly_aiui/include \
	-I../../../../apps/common/LLM/duer/intelligent_duer \
	-I../../../../apps/common/LLM/duer/my_platform \
	-I../../../../include_lib/net/network_download \
	-I../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/inc \
	-I../../../../apps/common/LLM/tc_iot_twetalk/twetalk_ws \
	-I../../../../apps/wifi_story_machine/ai_sdk \
	-I../../../../apps/wifi_story_machine/ai_gizwits \
	-I../../../../apps/wifi_story_machine/ai_utils \
	-I../../../../apps/common/nertc_sdk/include \

# 需要编译的 .c 文件
c_SRC_FILES := \
	../../../../apps/wifi_story_machine/ai_sdk/ai_coze_api_message.c \
	../../../../apps/wifi_story_machine/ai_sdk/ai_coze_audio_drive.c \
	../../../../apps/wifi_story_machine/ai_sdk/ai_coze_server.c \
	../../../../apps/wifi_story_machine/ai_sdk/ai_coze_websocket.c \
	../../../../apps/wifi_story_machine/ai_sdk/nertc_protocol.c \
	../../../../apps/wifi_story_machine/ai_sdk/ai_device_manager.c \
	../../../../apps/wifi_story_machine/ai_gizwits/gizwits_login.c \
	../../../../apps/wifi_story_machine/ai_gizwits/gizwits_mqtt.c \
	../../../../apps/wifi_story_machine/ai_utils/ringbuffer.c \
	../../../../apps/wifi_story_machine/ai_utils/app_md5.c \
	../../../../apps/wifi_story_machine/ai_utils/app_sha_256.c \
	../../../../apps/wifi_story_machine/ai_utils/le_gizwits_net_cfg.c \
	../../../../apps/wifi_story_machine/ai_utils/app_firmware_download.c \
	../../../../apps/wifi_story_machine/ai_utils/wifi_prov_core.c \
	../../../../apps/wifi_story_machine/app_music_coze.c \
	../../../../apps/common/LLM/QYAI/net_music.c \
	../../../../apps/common/LLM/QYAI/qyai_instruct_word.c \
	../../../../apps/common/LLM/QYAI/qyai_pcm.c \
	../../../../apps/common/LLM/QYAI/qyai_platform.c \
	../../../../apps/common/LLM/aitoye2e/e2e_demo.c \
	../../../../apps/common/LLM/audio/audio_input.c \
	../../../../apps/common/LLM/duer/intelligent_duer/duer_auth_algorithm.c \
	../../../../apps/common/LLM/duer/intelligent_duer/duer_http_req.c \
	../../../../apps/common/LLM/duer/intelligent_duer/duer_json_parse.c \
	../../../../apps/common/LLM/duer/intelligent_duer/duer_json_request.c \
	../../../../apps/common/LLM/duer/intelligent_duer/duer_record.c \
	../../../../apps/common/LLM/duer/intelligent_duer/duer_socket.c \
	../../../../apps/common/LLM/duer/intelligent_duer/test_demo.c \
	../../../../apps/common/LLM/duer/my_platform/my_platform_http.c \
	../../../../apps/common/LLM/duer/my_platform/my_platform_http_api.c \
	../../../../apps/common/LLM/duer/my_platform/my_platform_list_download.c \
	../../../../apps/common/LLM/duer/my_platform/my_platform_record.c \
	../../../../apps/common/LLM/duer/my_platform/my_platform_url_list.c \
	../../../../apps/common/LLM/extra/g711.c \
	../../../../apps/common/LLM/ifly_aiui/ifly_aiui_demo.c \
	../../../../apps/common/LLM/ifly_aiui/ifly_aiui_user.c \
	../../../../apps/common/LLM/ifly_aiui/ifly_model_parse.c \
	../../../../apps/common/LLM/onesdk/examples/onesdk_realtime/chat_agent/realtime_demo.c \
	../../../../apps/common/LLM/onesdk/platform/plat/alloc.c \
	../../../../apps/common/LLM/onesdk/platform/plat/platform.c \
	../../../../apps/common/LLM/onesdk/platform/plat/util.c \
	../../../../apps/common/LLM/onesdk/src/aigw/auth.c \
	../../../../apps/common/LLM/onesdk/src/aigw/llm.c \
	../../../../apps/common/LLM/onesdk/src/aws/common/allocator.c \
	../../../../apps/common/LLM/onesdk/src/aws/common/array_list.c \
	../../../../apps/common/LLM/onesdk/src/aws/common/assert.c \
	../../../../apps/common/LLM/onesdk/src/aws/common/atomics.c \
	../../../../apps/common/LLM/onesdk/src/aws/common/byte_buf.c \
	../../../../apps/common/LLM/onesdk/src/aws/common/byte_order.c \
	../../../../apps/common/LLM/onesdk/src/aws/common/clock.c \
	../../../../apps/common/LLM/onesdk/src/aws/common/common.c \
	../../../../apps/common/LLM/onesdk/src/aws/common/data_time.c \
	../../../../apps/common/LLM/onesdk/src/aws/common/encoding.c \
	../../../../apps/common/LLM/onesdk/src/aws/common/error.c \
	../../../../apps/common/LLM/onesdk/src/aws/common/hash_table.c \
	../../../../apps/common/LLM/onesdk/src/aws/common/json.c \
	../../../../apps/common/LLM/onesdk/src/aws/common/linked_list.c \
	../../../../apps/common/LLM/onesdk/src/aws/common/math.c \
	../../../../apps/common/LLM/onesdk/src/aws/common/string.c \
	../../../../apps/common/LLM/onesdk/src/aws/common/time.c \
	../../../../apps/common/LLM/onesdk/src/aws/common/zero.c \
	../../../../apps/common/LLM/onesdk/src/infer_inner_chat.c \
	../../../../apps/common/LLM/onesdk/src/infer_realtime_ws.c \
	../../../../apps/common/LLM/onesdk/src/iot/dynreg.c \
	../../../../apps/common/LLM/onesdk/src/iot/iot_kv.c \
	../../../../apps/common/LLM/onesdk/src/iot/iot_mqtt.c \
	../../../../apps/common/LLM/onesdk/src/iot/iot_popen.c \
	../../../../apps/common/LLM/onesdk/src/iot/iot_utils.c \
	../../../../apps/common/LLM/onesdk/src/iot/log/iot_log.c \
	../../../../apps/common/LLM/onesdk/src/iot/log/iot_log_report.c \
	../../../../apps/common/LLM/onesdk/src/iot/ota/iot_ota_api.c \
	../../../../apps/common/LLM/onesdk/src/iot/ota/iot_ota_utils.c \
	../../../../apps/common/LLM/onesdk/src/iot/thingmodel/custom_topic.c \
	../../../../apps/common/LLM/onesdk/src/iot/thingmodel/device_delay.c \
	../../../../apps/common/LLM/onesdk/src/iot/thingmodel/event.c \
	../../../../apps/common/LLM/onesdk/src/iot/thingmodel/gateway.c \
	../../../../apps/common/LLM/onesdk/src/iot/thingmodel/iot_ntp.c \
	../../../../apps/common/LLM/onesdk/src/iot/thingmodel/iot_tm_api.c \
	../../../../apps/common/LLM/onesdk/src/iot/thingmodel/property.c \
	../../../../apps/common/LLM/onesdk/src/iot/thingmodel/service.c \
	../../../../apps/common/LLM/onesdk/src/iot/thingmodel/shadow.c \
	../../../../apps/common/LLM/onesdk/src/iot/thingmodel/webshell.c \
	../../../../apps/common/LLM/onesdk/src/iot_basic.c \
	../../../../apps/common/LLM/onesdk/src/onesdk.c \
	../../../../apps/common/LLM/onesdk/src/onesdk_chat.c \
	../../../../apps/common/LLM/onesdk/src/onesdk_iot.c \
	../../../../apps/common/LLM/onesdk/src/onesdk_rt.c \
	../../../../apps/common/LLM/onesdk/src/protocols/http.c \
	../../../../apps/common/LLM/onesdk/src/protocols/http_libs.c \
	../../../../apps/common/LLM/onesdk/src/protocols/lws_http_client.c \
	../../../../apps/common/LLM/onesdk/src/protocols/lws_http_download.c \
	../../../../apps/common/LLM/onesdk/src/protocols/lws_http_upload.c \
	../../../../apps/common/LLM/onesdk/src/util/aes.c \
	../../../../apps/common/LLM/onesdk/src/util/aes_decode.c \
	../../../../apps/common/LLM/onesdk/src/util/aws_json.c \
	../../../../apps/common/LLM/onesdk/src/util/hmac_sha256.c \
	../../../../apps/common/LLM/onesdk/src/util/md5.c \
	../../../../apps/common/LLM/onesdk/src/util/sha256.c \
	../../../../apps/common/LLM/onesdk/src/util/util.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/port/HAL_File_free_rtos.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/port/HAL_OS_free_rtos.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/port/HAL_SoftAP_ac79.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/port/HAL_TCP_free_rtos.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/port/HAL_Timer_free_rtos.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/port/HAL_UDP_free_rtos.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/port/HAL_nvs_ac79.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/port/HAL_ota_ac79.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/port/HAL_wifi_ac79.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/port/qcloud_iot_link_ble_ac79.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/ble_llsync_device_info.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/ble_llsync_service.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/cos_download.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/data_template.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/data_template_action.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/data_template_cloud.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/data_template_config.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/data_template_event.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/data_template_mqtt.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/data_template_parse.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/data_template_property.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/dynreg_request.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/dynreg_signature.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/http_ca.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/http_client.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/http_signed.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/mqtt_client.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/mqtt_client_common.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/mqtt_client_connect.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/mqtt_client_publish.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/mqtt_client_subscribe.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/mqtt_client_yield.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/mqtt_packet_deserialize.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/mqtt_packet_serialize.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/network_interface.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/ota_mqtt.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/qcloud_iot_device.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/qcloud_iot_link.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/qcloud_iot_timer.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/qcloud_iot_tls_client.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/sevice_mqtt.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/system_mqtt.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/utils_aes.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/utils_base64.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/utils_crc.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/utils_downloader.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/utils_hmac.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/utils_json.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/utils_list.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/utils_log.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/utils_md5.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/utils_min_heap.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/utils_ringbuffer.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/utils_sha1.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/utils_sha256.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/utils_signature.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/wifi_bind.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/wifi_config.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/wifi_llsync.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/wifi_softap.c \
	../../../../apps/common/LLM/tc_iot_twetalk/qcloud_iot_explorer_sdk/src/wifi_udp.c \
	../../../../apps/common/LLM/tc_iot_twetalk/twetalk_ws/data_template_app.c \
	../../../../apps/common/LLM/tc_iot_twetalk/twetalk_ws/ota_downloader.c \
	../../../../apps/common/LLM/tc_iot_twetalk/twetalk_ws/test_audio_opus.c \
	../../../../apps/common/LLM/tc_iot_twetalk/twetalk_ws/twetalk_app.c \
	../../../../apps/common/LLM/volc/VolcEngineRTCLiteDemo.c \
	../../../../apps/common/LLM/volc/access_token.c \
	../../../../apps/common/LLM/volc/net_request.c \
	../../../../apps/common/LLM/volc/src/common/volc_crypto.c \
	../../../../apps/common/LLM/volc/src/common/volc_sprintf.c \
	../../../../apps/common/LLM/volc/src/common/volc_string.c \
	../../../../apps/common/LLM/volc/src/common/volc_tls.c \
	../../../../apps/common/LLM/volc/src/platform/volc_atomic.c \
	../../../../apps/common/LLM/volc/src/platform/volc_device.c \
	../../../../apps/common/LLM/volc/src/platform/volc_device_finger_print.c \
	../../../../apps/common/LLM/volc/src/platform/volc_directory.c \
	../../../../apps/common/LLM/volc/src/platform/volc_errno.c \
	../../../../apps/common/LLM/volc/src/platform/volc_fileio.c \
	../../../../apps/common/LLM/volc/src/platform/volc_log.c \
	../../../../apps/common/LLM/volc/src/platform/volc_memory.c \
	../../../../apps/common/LLM/volc/src/platform/volc_mutex.c \
	../../../../apps/common/LLM/volc/src/platform/volc_network.c \
	../../../../apps/common/LLM/volc/src/platform/volc_socket.c \
	../../../../apps/common/LLM/volc/src/platform/volc_thread.c \
	../../../../apps/common/LLM/volc/src/platform/volc_time.c \
	../../../../apps/common/LLM/volc/src/platform/volc_uuid.c \
	../../../../apps/common/LLM/volc/src/platform/volc_zlib.c \
	../../../../apps/common/asr/aisp/aisp.c \
	../../../../apps/common/asr/jlkws/jlkws.c \
	../../../../apps/common/asr/roobo/roobo.c \
	../../../../apps/common/asr/wanson/wanson.c \
	../../../../apps/common/audio_music/audio_config.c \
	../../../../apps/common/audio_music/audio_eff_default_parm.c \
	../../../../apps/common/audio_music/effects_adj.c \
	../../../../apps/common/audio_music/eq_config_new.c \
	../../../../apps/common/audio_music/http_recorder.c \
	../../../../apps/common/audio_music/midi/audio_dec_midi_ctrl.c \
	../../../../apps/common/audio_music/music_decrypt.c \
	../../../../apps/common/audio_music/pcm_play_api.c \
	../../../../apps/common/audio_music/usb_audio_api.c \
	../../../../apps/common/audio_music/wifi_upload_pcm.c \
	../../../../apps/common/ble/le_24g_deal.c \
	../../../../apps/common/ble/le_hogp.c \
	../../../../apps/common/ble/le_net_central.c \
	../../../../apps/common/ble/le_net_cfg.c \
	../../../../apps/common/ble/le_net_cfg_dui.c \
	../../../../apps/common/ble/le_net_cfg_qyai.c \
	../../../../apps/common/ble/le_net_cfg_tencent.c \
	../../../../apps/common/ble/le_net_cfg_turing.c \
	../../../../apps/common/ble/le_trans_data.c \
	../../../../apps/common/ble/mesh/examples/AliGenie_fan.c \
	../../../../apps/common/ble/mesh/examples/AliGenie_light.c \
	../../../../apps/common/ble/mesh/examples/AliGenie_socket.c \
	../../../../apps/common/ble/mesh/examples/TUYA_light.c \
	../../../../apps/common/ble/mesh/examples/generic_onoff_client.c \
	../../../../apps/common/ble/mesh/examples/generic_onoff_server.c \
	../../../../apps/common/ble/mesh/examples/onoff_tobe_provision.c \
	../../../../apps/common/ble/mesh/examples/provisioner.c \
	../../../../apps/common/ble/mesh/examples/vendor_client.c \
	../../../../apps/common/ble/mesh/examples/vendor_server.c \
	../../../../apps/common/ble/mesh/mesh_config_common.c \
	../../../../apps/common/ble/mesh/model_api.c \
	../../../../apps/common/ble/mesh/unix_timestamp.c \
	../../../../apps/common/ble/multi_demo/le_multi_client.c \
	../../../../apps/common/ble/multi_demo/le_multi_common.c \
	../../../../apps/common/ble/multi_demo/le_multi_trans.c \
	../../../../apps/common/c++/cpp_run_init.c \
	../../../../apps/common/camera/byd20a6/byd20a6.c \
	../../../../apps/common/camera/byd30a2/byd30a2.c \
	../../../../apps/common/camera/byd3720/byd3720.c \
	../../../../apps/common/camera/byd3901/byd3901.c \
	../../../../apps/common/camera/byd3a03/byd3a03.c \
	../../../../apps/common/camera/gc0307_dvp/gc0307.c \
	../../../../apps/common/camera/gc0308_dvp/gc0308.c \
	../../../../apps/common/camera/gc0309_dvp/gc0309.c \
	../../../../apps/common/camera/gc0310_spi/gc0310.c \
	../../../../apps/common/camera/gc0312_dvp/gc0312.c \
	../../../../apps/common/camera/gc0328_dvp/gc0328.c \
	../../../../apps/common/camera/gc0329_dvp/gc0329.c \
	../../../../apps/common/camera/gc032a_dvp/gc032a.c \
	../../../../apps/common/camera/gc2035_dvp/gc2035.c \
	../../../../apps/common/camera/gc2145_dvp/gc2145.c \
	../../../../apps/common/camera/h7375s/h7375s.c \
	../../../../apps/common/camera/ov7670_dvp/ov7670.c \
	../../../../apps/common/camera/pr2000k/pr2000k.c \
	../../../../apps/common/camera/xc7011_dvp/xc7011.c \
	../../../../apps/common/config/bt_profile_config.c \
	../../../../apps/common/config/log_config/app_config.c \
	../../../../apps/common/config/log_config/lib_btctrler_config.c \
	../../../../apps/common/config/log_config/lib_btstack_config.c \
	../../../../apps/common/config/log_config/lib_driver_config.c \
	../../../../apps/common/config/log_config/lib_system_config.c \
	../../../../apps/common/config/log_config/lib_update_config.c \
	../../../../apps/common/config/new_cfg_tool.c \
	../../../../apps/common/config/user_cfg.c \
	../../../../apps/common/debug/debug.c \
	../../../../apps/common/debug/debug_user.c \
	../../../../apps/common/example/third_party/littlefs/lfs.c \
	../../../../apps/common/example/third_party/littlefs/lfs_test.c \
	../../../../apps/common/example/third_party/littlefs/lfs_util.c \
	../../../../apps/common/example/third_party/littlefs/port/wl82/port.c \
	../../../../apps/common/fat_nor/extflash.c \
	../../../../apps/common/fm/fm_manage.c \
	../../../../apps/common/fm/fm_rw.c \
	../../../../apps/common/fm/qn8035/QN8035.c \
	../../../../apps/common/fm/rda5807/RDA5807.c \
	../../../../apps/common/iic/iic.c \
	../../../../apps/common/iic/software_iic.c \
	../../../../apps/common/jl_math/flfft_core_pi32v2.c \
	../../../../apps/common/jl_math/jl_fft.c \
	../../../../apps/common/jl_math/jl_math.c \
	../../../../apps/common/jl_math/nn_function_vec.c \
	../../../../apps/common/key/adkey.c \
	../../../../apps/common/key/adkey_rtcvdd.c \
	../../../../apps/common/key/ctmu_touch_key.c \
	../../../../apps/common/key/iokey.c \
	../../../../apps/common/key/irkey.c \
	../../../../apps/common/key/key_driver.c \
	../../../../apps/common/key/matrix_keyboard.c \
	../../../../apps/common/key/rdec_key.c \
	../../../../apps/common/key/slidekey.c \
	../../../../apps/common/key/tent600_key.c \
	../../../../apps/common/key/touch_key.c \
	../../../../apps/common/key/uart_key.c \
	../../../../apps/common/net/assign_macaddr.c \
	../../../../apps/common/net/config_network.c \
	../../../../apps/common/net/iperf_test.c \
	../../../../apps/common/net/platform_cfg.c \
	../../../../apps/common/net/testbox/client.c \
	../../../../apps/common/net/testbox/fsm.c \
	../../../../apps/common/net/testbox/wbcp.c \
	../../../../apps/common/net/voiceprint_cfg.c \
	../../../../apps/common/net/wifi_conf.c \
	../../../../apps/common/rf_fcc_tool/bt.c \
	../../../../apps/common/rf_fcc_tool/rf_fcc_main.c \
	../../../../apps/common/system/device_mount.c \
	../../../../apps/common/system/flash_write_protect.c \
	../../../../apps/common/system/init.c \
	../../../../apps/common/system/power_manage.c \
	../../../../apps/common/system/sdram_config_save_flash.c \
	../../../../apps/common/system/system_reset_reason.c \
	../../../../apps/common/system/system_vdd_clock.c \
	../../../../apps/common/system/version.c \
	../../../../apps/common/ui/GUI_GIF.c \
	../../../../apps/common/ui/font/fontinit.c \
	../../../../apps/common/ui/gif_api.c \
	../../../../apps/common/ui/lcd/lcd_ui_api.c \
	../../../../apps/common/ui/lcd_driver/lcd_480x272_8bits.c \
	../../../../apps/common/ui/lcd_driver/lcd_data_driver.c \
	../../../../apps/common/ui/lcd_driver/lcd_drive.c \
	../../../../apps/common/ui/lcd_driver/lcd_hx8357.c \
	../../../../apps/common/ui/lcd_driver/lcd_ili9341.c \
	../../../../apps/common/ui/lcd_driver/lcd_ili9481.c \
	../../../../apps/common/ui/lcd_driver/lcd_ili9488.c \
	../../../../apps/common/ui/lcd_driver/lcd_st7735s.c \
	../../../../apps/common/ui/lcd_driver/lcd_st7789s.c \
	../../../../apps/common/ui/lcd_driver/lcd_st7789v.c \
	../../../../apps/common/ui/lcd_driver/lcd_tm9486.c \
	../../../../apps/common/ui/touch_panel/ft6236.c \
	../../../../apps/common/ui/touch_panel/gt911.c \
	../../../../apps/common/ui/ui_action_video.c \
	../../../../apps/common/ui/ui_driver/interface/ui_platform.c \
	../../../../apps/common/update/fs_update.c \
	../../../../apps/common/update/http_update.c \
	../../../../apps/common/update/net_update.c \
	../../../../apps/common/update/update.c \
	../../../../apps/common/usb/device/cdc.c \
	../../../../apps/common/usb/device/descriptor.c \
	../../../../apps/common/usb/device/hid.c \
	../../../../apps/common/usb/device/msd.c \
	../../../../apps/common/usb/device/msd_upgrade.c \
	../../../../apps/common/usb/device/printer.c \
	../../../../apps/common/usb/device/ram_disk_circle.c \
	../../../../apps/common/usb/device/ram_disk_update.c \
	../../../../apps/common/usb/device/task_pc.c \
	../../../../apps/common/usb/device/uac.c \
	../../../../apps/common/usb/device/uac2.c \
	../../../../apps/common/usb/device/uac_stream.c \
	../../../../apps/common/usb/device/usb_device.c \
	../../../../apps/common/usb/device/user_setup.c \
	../../../../apps/common/usb/device/uvc.c \
	../../../../apps/common/usb/host/adb.c \
	../../../../apps/common/usb/host/aoa.c \
	../../../../apps/common/usb/host/audio.c \
	../../../../apps/common/usb/host/hid.c \
	../../../../apps/common/usb/host/usb_bulk_transfer.c \
	../../../../apps/common/usb/host/usb_ctrl_transfer.c \
	../../../../apps/common/usb/host/usb_host.c \
	../../../../apps/common/usb/host/usb_host_cdc.c \
	../../../../apps/common/usb/host/usb_storage.c \
	../../../../apps/common/usb/host/usb_video.c \
	../../../../apps/common/usb/host/usbnet.c \
	../../../../apps/common/usb/host/uvc_host.c \
	../../../../apps/common/usb/usb_config.c \
	../../../../apps/common/usb/usb_epbuf_manager.c \
	../../../../apps/common/usb/usb_host_config.c \
	../../../../apps/wifi_story_machine/app_main.c \
	../../../../apps/wifi_story_machine/app_music.c \
	../../../../apps/wifi_story_machine/app_music_dui.c \
	../../../../apps/wifi_story_machine/app_music_tencent.c \
	../../../../apps/wifi_story_machine/board/wl82/board_7911B.c \
	../../../../apps/wifi_story_machine/board/wl82/board_7911D.c \
	../../../../apps/wifi_story_machine/board/wl82/board_7912D.c \
	../../../../apps/wifi_story_machine/board/wl82/board_7913A.c \
	../../../../apps/wifi_story_machine/board/wl82/board_7916A.c \
	../../../../apps/wifi_story_machine/bt_ble/ble.c \
	../../../../apps/wifi_story_machine/bt_ble/bt_decode.c \
	../../../../apps/wifi_story_machine/bt_ble/bt_emitter.c \
	../../../../apps/wifi_story_machine/bt_ble/bt_music.c \
	../../../../apps/wifi_story_machine/multiple_ethernetif.c \
	../../../../apps/wifi_story_machine/ui/led_ui_gr202.c \
	../../../../apps/wifi_story_machine/video/camera_lcd_demo.c \
	../../../../apps/wifi_story_machine/video/get_yuv_data.c \
	../../../../apps/wifi_story_machine/video/jpeg2yuv.c \
	../../../../apps/wifi_story_machine/video/network_mssdp.c \
	../../../../apps/wifi_story_machine/video/spi_data_deal.c \
	../../../../apps/wifi_story_machine/video/spi_recv.c \
	../../../../apps/wifi_story_machine/video/stream/video_rt_usr.c \
	../../../../apps/wifi_story_machine/video/stream_protocol.c \
	../../../../apps/wifi_story_machine/video/strm_video_rec.c \
	../../../../apps/wifi_story_machine/video/user_video_rec.c \
	../../../../apps/wifi_story_machine/wifi_app_task.c \
	../../../../cpu/wl82/debug.c \
	../../../../cpu/wl82/iic.c \
	../../../../cpu/wl82/key/adc_api.c \
	../../../../cpu/wl82/key/ctmu.c \
	../../../../cpu/wl82/key/irflt.c \
	../../../../cpu/wl82/key/plcnt.c \
	../../../../cpu/wl82/key/rdec.c \
	../../../../cpu/wl82/port_waked_up.c \
	../../../../cpu/wl82/setup.c \


# 需要编译的 .S 文件
S_SRC_FILES := \
	../../../../include_lib/btctrler/version.z.S \
	../../../../include_lib/btstack/version.z.S \
	../../../../include_lib/driver/version.z.S \
	../../../../include_lib/media/version.z.S \
	../../../../include_lib/net/version.z.S \
	../../../../include_lib/server/version.z.S \
	../../../../include_lib/system/version.z.S \
	../../../../include_lib/update/version.z.S \
	../../../../include_lib/utils/version.z.S \
	../../../../lib/net/version.z.S \
	../../../../lib/server/version.z.S \
	../../../../lib/utils/version.z.S \


# 需要编译的 .s 文件
s_SRC_FILES :=


# 需要编译的 .cpp 文件
cpp_SRC_FILES := \
	../../../../apps/common/c++/cxx_runtime.cpp \

# nertc_sdk 预编译库路径
NERTC_SDK_LIB := ../../../../apps/common/nertc_sdk/lib/ac7911/libnertc_sdk.a


# 需要编译的 .cc 文件
cc_SRC_FILES :=


# 需要编译的 .cxx 文件
cxx_SRC_FILES :=


# 链接参数
LFLAGS := \
	--plugin-opt=-pi32v2-always-use-itblock=false \
	--plugin-opt=-enable-ipra=true \
	--plugin-opt=-pi32v2-merge-max-offset=4096 \
	--plugin-opt=-pi32v2-enable-simd=true \
	--plugin-opt=mcpu=r3 \
	--plugin-opt=-global-merge-on-const \
	--plugin-opt=-inline-threshold=5 \
	--plugin-opt=-inline-max-allocated-size=32 \
	--plugin-opt=-inline-normal-into-special-section=true \
	--plugin-opt=-dont-used-symbol-list=malloc,free,sprintf,printf,puts,putchar \
	--plugin-opt=save-temps \
	--plugin-opt=-pi32v2-enable-rep-memop \
	--plugin-opt=-emulated-tls \
	--sort-common \
	--plugin-opt=-used-symbol-file=../../../../cpu/wl82/sdk_used_list.used \
	--plugin-opt=-enable-movable-region=true \
	--plugin-opt=-movable-region-section-prefix=.movable.slot. \
	--plugin-opt=-movable-region-stub-section-prefix=.movable.stub. \
	--plugin-opt=-movable-region-prefix=.movable.region. \
	--plugin-opt=-movable-region-stub-prefix=__movable_stub_ \
	--plugin-opt=-movable-region-stub-swi-number=-2 \
	--plugin-opt=-movable-region-map-file-list=../../../../apps/common/movable/funcname.txt \
	--plugin-opt=-movable-region-section-map-file-list=../../../../apps/common/movable/section.txt \
	--plugin-opt=-movable-region-exclude-func-file-list=../../../../apps/common/movable/exclude.txt \
	--plugin-opt=-pi32v2-large-program=true \
	--start-group \
	../../../../include_lib/newlib/pi32v2-lib/libm.a \
	../../../../include_lib/newlib/pi32v2-lib/libc.a \
	../../../../include_lib/newlib/pi32v2-lib/libcompiler_rt.a \
	../../../../include_lib/c++/libstdc++/libc++.a \
	../../../../include_lib/c++/libstdc++/libc++abi.a \
	../../../../include_lib/c++/libstdc++/libemutls.a \
	--start-group  \
	../../../../cpu/wl82/liba/event.a \
	../../../../cpu/wl82/liba/system.a \
	../../../../cpu/wl82/liba/fs.a \
	../../../../cpu/wl82/liba/cfg_tool.a \
	../../../../cpu/wl82/liba/cpu.a \
	../../../../cpu/wl82/liba/update.a \
	../../../../cpu/wl82/liba/common_lib.a \
	../../../../cpu/wl82/liba/wl_rf_common.a \
	../../../../cpu/wl82/liba/VirtualBass.a \
	../../../../cpu/wl82/liba/lib_esco_repair.a \
	../../../../cpu/wl82/liba/audio_server.a \
	../../../../cpu/wl82/liba/echo_server.a \
	../../../../cpu/wl82/liba/led_ui_server.a \
	../../../../cpu/wl82/liba/lib_opus_enc.a \
	../../../../cpu/wl82/liba/lib_opus_stenc.a \
	../../../../cpu/wl82/liba/libspeex.a \
	../../../../cpu/wl82/liba/libvad.a \
	../../../../cpu/wl82/liba/lib_wma_dec.a \
	../../../../cpu/wl82/liba/lib_ape_dec.a \
	../../../../cpu/wl82/liba/lib_flac_dec.a \
	../../../../cpu/wl82/liba/lib_mp3_dec.a \
	../../../../cpu/wl82/liba/lib_mp3_enc.a \
	../../../../cpu/wl82/liba/lib_wav_dec.a \
	../../../../cpu/wl82/liba/lib_dts_dec.a \
	../../../../cpu/wl82/liba/lib_opus_dec.a \
	../../../../cpu/wl82/liba/lib_amr_dec.a \
	../../../../cpu/wl82/liba/lib_amr_enc.a \
	../../../../cpu/wl82/liba/lib_amrwb_enc.a \
	../../../../cpu/wl82/liba/lib_adpcm_enc.a \
	../../../../cpu/wl82/liba/lib_pitchshifter.a \
	../../../../cpu/wl82/liba/lib_rmono2stereo.a \
	../../../../cpu/wl82/liba/lib_reverb_cal.a \
	../../../../cpu/wl82/liba/lib_howling.a \
	../../../../cpu/wl82/liba/lib_pitch_speed.a \
	../../../../cpu/wl82/liba/lib_roobo_fig.a \
	../../../../cpu/wl82/liba/lib_roobo_aec.a \
	../../../../cpu/wl82/liba/lib_wanson_asr.a \
	../../../../cpu/wl82/liba/libaec.a \
	../../../../cpu/wl82/liba/libSMS_TDE_pi32v2_OnChip.a \
	../../../../cpu/wl82/liba/libFFT_pi32v2_OnChip.a \
	../../../../cpu/wl82/liba/lib_dvad.a \
	../../../../cpu/wl82/liba/libdns.a \
	../../../../cpu/wl82/liba/libjlsp.a \
	../../../../cpu/wl82/liba/libjlsp_kws_far_keyword.a \
	../../../../cpu/wl82/liba/libkwscommon.a \
	../../../../cpu/wl82/liba/lib_usb_syn.a \
	../../../../cpu/wl82/liba/lib_spectrum_show.a \
	../../../../cpu/wl82/liba/img_stitch.a \
	../../../../cpu/wl82/liba/lib_ogg_vorbis_dec.a \
	../../../../cpu/wl82/liba/lib_midi_dec.a \
	../../../../cpu/wl82/liba/media_app.a \
	../../../../cpu/wl82/liba/libcompressor.a \
	../../../../cpu/wl82/liba/lib_crossover_coff.a \
	../../../../cpu/wl82/liba/libdrc.a \
	../../../../cpu/wl82/liba/net_server.a \
	../../../../cpu/wl82/liba/video_rec_server.a \
	../../../../cpu/wl82/liba/aisIntlib_single_xiaoai.a \
	../../../../cpu/wl82/liba/libaisp_single.a \
	../../../../cpu/wl82/liba/libauth_aisp.a \
	../../../../cpu/wl82/liba/libauth.a \
	../../../../cpu/wl82/liba/lib_m4a_dec.a \
	../../../../cpu/wl82/liba/lwip_2_2_0.a \
	../../../../cpu/wl82/liba/wl_wifi_sta.a \
	../../../../cpu/wl82/liba/lte_module.a \
	../../../../cpu/wl82/liba/ai_server.a \
	../../../../cpu/wl82/liba/zliblite.a \
	../../../../cpu/wl82/liba/profile.a \
	../../../../cpu/wl82/liba/stream_media_server.a \
	../../../../cpu/wl82/liba/libmbedtls_3_4_0.a \
	../../../../cpu/wl82/liba/wpasupplicant.a \
	../../../../cpu/wl82/liba/hsm.a \
	../../../../cpu/wl82/liba/http_cli.a \
	../../../../cpu/wl82/liba/https_cli.a \
	../../../../cpu/wl82/liba/http_server.a \
	../../../../cpu/wl82/liba/json.a \
	../../../../cpu/wl82/liba/libduer-device.a \
	../../../../cpu/wl82/liba/network_download.a \
	../../../../cpu/wl82/liba/lib_mqtt.a \
	../../../../cpu/wl82/liba/lib_sound_wave.a \
	../../../../cpu/wl82/liba/libturing.a \
	../../../../cpu/wl82/liba/libdui.a \
	../../../../cpu/wl82/liba/libJlCloud.a \
	../../../../cpu/wl82/liba/libecho_cloud.a \
	../../../../cpu/wl82/liba/libmpeg.a \
	../../../../cpu/wl82/liba/libdcl.a \
	../../../../cpu/wl82/liba/libdlna.a \
	../../../../cpu/wl82/liba/libqrcode.a \
	../../../../cpu/wl82/liba/libwt.a \
	../../../../cpu/wl82/liba/libvtbk.a \
	../../../../cpu/wl82/liba/libtencent.a \
	../../../../cpu/wl82/liba/websocket.a \
	../../../../cpu/wl82/liba/iperf.a \
	../../../../cpu/wl82/liba/tmallgenie.a \
	../../../../cpu/wl82/liba/tmallgenie_todo.a \
	../../../../cpu/wl82/liba/cJSON.a \
	../../../../cpu/wl82/liba/ftpserver_new.a \
	../../../../cpu/wl82/liba/http_server.a  \
	../../../../cpu/wl82/liba/btctrler.a \
	../../../../cpu/wl82/liba/btstack.a \
	../../../../cpu/wl82/liba/rcsp_stack.a \
	../../../../cpu/wl82/liba/lib_ccm_aes.a \
	../../../../cpu/wl82/liba/lib_sbc_eng.a \
	../../../../cpu/wl82/liba/lib_sig_mesh.a \
	../../../../cpu/wl82/liba/crypto_toolbox_Osize.a \
	--end-group \
	../../../../cpu/wl82/liba/video_dec_server.a \
	../../../../cpu/wl82/liba/ui.a \
	../../../../cpu/wl82/liba/res.a \
	../../../../cpu/wl82/liba/font.a \
	../../../../cpu/wl82/liba/ui_draw.a \
	../../../../cpu/wl82/liba/libVolcEngineRTCLite.a \
	../../../../cpu/wl82/liba/libcurl.a \
	../../../../cpu/wl82/liba/libwebsockets.a \
	../../../../cpu/wl82/liba/cJSON.a  \
	../../../../cpu/wl82/liba/libmbedtls_3_4_0.a  \
	../../../../cpu/wl82/liba/sxy_qyai.a \
	../../../../cpu/wl82/liba/libe2e.a \
	../../../../cpu/wl82/liba/websocket.a  \
	../../../../cpu/wl82/liba/libifly_aiui_lib.a \
	../../../../cpu/wl82/liba/websocket.a   \
	../../../../cpu/wl82/liba/cJSON.a   \
	../../../../cpu/wl82/liba/lib_twetalk.a \
	$(NERTC_SDK_LIB) \
	-T../../../../cpu/wl82/sdk.ld \
	-M=../../../../cpu/wl82/tools/sdk.map \
	--end-group  \
	--plugin-opt=mcpu=r3 \
	--plugin-opt=-mattr=+fprev1 \



c_OBJS    := $(c_SRC_FILES:%.c=%.c.o)
S_OBJS    := $(S_SRC_FILES:%.S=%.S.o)
s_OBJS    := $(s_SRC_FILES:%.s=%.s.o)
cpp_OBJS  := $(cpp_SRC_FILES:%.cpp=%.cpp.o)
cxx_OBJS  := $(cxx_SRC_FILES:%.cxx=%.cxx.o)
cc_OBJS   := $(cc_SRC_FILES:%.cc=%.cc.o)

OBJS      := $(c_OBJS) $(S_OBJS) $(s_OBJS) $(cpp_OBJS) $(cxx_OBJS) $(cc_OBJS)
DEP_FILES := $(OBJS:%.o=%.d)


OBJS      := $(addprefix $(BUILD_DIR)/, $(OBJS:$(ROOT_PREFIX)/%=%))
DEP_FILES := $(addprefix $(BUILD_DIR)/, $(DEP_FILES:$(ROOT_PREFIX)/%=%))

# nertc_sdk: 不再将 .o 直接加入 OBJS，改为链接 .a 静态库
# OBJS      += $(NERTC_SDK_OBJS)


VERBOSE ?= 0
ifeq ($(VERBOSE), 1)
QUITE :=
else
QUITE := @
endif

# 一些旧的 make 不支持 file 函数，需要 make 的时候指定 LINK_AT=0 make
LINK_AT ?= 1

# 表示下面的不是一个文件的名字，无论是否存在 all, clean, pre_build 这样的文件
# 还是要执行命令
# see: https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean pre_build

# 不要使用 make 预设置的规则
# see: https://www.gnu.org/software/make/manual/html_node/Suffix-Rules.html
.SUFFIXES:

all: pre_build $(OUT_ELF)
	$(info +POST-BUILD)
	$(QUITE) $(RUN_POST_SCRIPT) sdk

pre_build:
	$(info +PRE-BUILD)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P ../../../../cpu/wl82/sdk_used_list.c -o ../../../../cpu/wl82/sdk_used_list.used
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P ../../../../apps/common/movable/section.c -o ../../../../apps/common/movable/section.txt
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P ../../../../cpu/wl82/sdk_ld.c -o ../../../../cpu/wl82/sdk.ld
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P ../../../../cpu/wl82/tools/download.c -o $(POST_SCRIPT)
	$(QUITE) $(FIXBAT) $(POST_SCRIPT)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P ../../../../cpu/wl82/tools/isd_config_rule.c -o ../../../../cpu/wl82/tools/isd_config.ini
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P ../../../../cpu/wl82/tools/isd_config_rule_loader.c -o ../../../../cpu/wl82/tools/isd_config_loader.ini

clean:
	$(QUITE) $(RM) $(OUT_ELF)
	$(QUITE) $(RM) $(BUILD_DIR)



ifeq ($(LINK_AT), 1)
$(OUT_ELF): $(OBJS) $(NERTC_SDK_LIB)
	$(info +LINK $@)
	$(shell $(MKDIR) $(@D))
	$(file >$(OBJ_FILE), $(OBJS))
	$(QUITE) $(LD) -o $(OUT_ELF) @$(OBJ_FILE) $(LFLAGS) $(LIBPATHS) $(LIBS)
else
$(OUT_ELF): $(OBJS) $(NERTC_SDK_LIB)
	$(info +LINK $@)
	$(shell $(MKDIR) $(@D))
	$(QUITE) $(LD) -o $(OUT_ELF) $(OBJS) $(LFLAGS) $(LIBPATHS) $(LIBS)
endif


$(BUILD_DIR)/%.c.o : $(ROOT_PREFIX)/%.c
	$(info +CC $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.S.o : $(ROOT_PREFIX)/%.S
	$(info +AS $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.s.o : $(ROOT_PREFIX)/%.s
	$(info +AS $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.cpp.o : $(ROOT_PREFIX)/%.cpp
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.cxx.o : $(ROOT_PREFIX)/%.cxx
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.cc.o : $(ROOT_PREFIX)/%.cc
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

-include $(DEP_FILES)
