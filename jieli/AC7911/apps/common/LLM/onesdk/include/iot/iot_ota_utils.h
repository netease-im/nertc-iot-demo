#ifndef ONESDK_IOT_IOT_OTA_UTILS_H
#define ONESDK_IOT_IOT_OTA_UTILS_H

enum ota_upgrade_device_status_enum {
    UpgradeDeviceStatusToUpgrade,
    UpgradeDeviceStatusDownloading,
    UpgradeDeviceStatusDownloaded,
    UpgradeDeviceStatusDiffRecovering,
    UpgradeDeviceStatusDiffRecovered,
    UpgradeDeviceStatusInstalling,
    UpgradeDeviceStatusInstalled,
    UpgradeDeviceStatusSuccess,
    UpgradeDeviceStatusFailed,
    UpgradeDeviceStatusCount,
};

const char *iot_ota_job_status_enum_to_string(enum ota_upgrade_device_status_enum status);

char *extract_filename(const char *url);

char *concat_const_strings(const char *str1, const char *str2);

int delete_fw_info_file(const char *file_name);

#endif //ONESDK_IOT_IOT_OTA_UTILS_H