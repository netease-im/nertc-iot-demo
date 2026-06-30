set(incs .)
set(srcs
    nertc.c
    nertc_main.c
    generic_mqtt.c
    nertc_ext_mqtt_bk.c
    nertc_ota.c
)

armino_component_register(
    SRCS "${srcs}"
    INCLUDE_DIRS "${incs}"
    PRIV_REQUIRES bk_common bk_boarding_service bk_smart_config bk_init lwip_intf_v2_1
                  mbedtls psa_mbedtls json multimedia avdk_utils nertc_sdk
                  video_engine audio_engine bk_app_event bk_factory_config ali_mqtt bk_cli
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE
    MBEDTLS_MD_CAN_SHA256=1
)

target_include_directories(${COMPONENT_LIB} PRIVATE
    $ENV{ARMINO_PATH}/components/lwip_intf_v2_1/lwip-2.1.2/src/include
    $ENV{ARMINO_PATH}/components/lwip_intf_v2_1/lwip-2.1.2/port
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error)
