#ifndef __AIUI_GET_FIRMWARE_H__
#define __AIUI_GET_FIRMWARE_H__

typedef struct aiui_upload_info_s {
    int update_way;
    char appid[16];
    char version[16];
    char url[128];
} aiui_upload_info_t;

int aiui_get_firmware_info(const char *appid, const char *deviceid, aiui_upload_info_t *info);
#endif // __AIUI_GET_FIRMWARE_H__