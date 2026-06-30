# BluFi App — 去硬件依赖后的精简构建

# 源文件：核心协议层 + 拆分的业务模块
set(SOURCES
    "blufi_app.cc"
    "blufi_hal.c"
    "blufi_provisioning.cc"
    "ota_upgrade.cc"
    "blufi_wifi.c"
    "blufi_init.c"
    "blufi_security.c"
    "settings.cc"
    "blufi_http_client.cc"
    "ota.cc"
)

# 头文件目录
set(INCLUDE_DIRS_LIST
    "."
)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDE_DIRS_LIST}
    REQUIRES bt esp_wifi mbedtls nvs_flash app_update json esp-tls esp_http_client efuse esp-wifi-connect esp_psram
)
